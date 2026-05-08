#include "chat_protocol.hpp"

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace asio = boost::asio;
namespace po = boost::program_options;
using boost::asio::ip::tcp;
using error_code = boost::system::error_code;

class chat_session;

class chat_room {
public:
    explicit chat_room(asio::io_context& io) : strand_(asio::make_strand(io)) {}

    void join(std::shared_ptr<chat_session> session, std::string announcement);
    void leave(std::shared_ptr<chat_session> session, std::string announcement);
    void broadcast(std::string message);

private:
    void broadcast_now(const std::string& message);

    asio::strand<asio::io_context::executor_type> strand_;
    std::set<std::shared_ptr<chat_session>> sessions_;
};

class chat_session : public std::enable_shared_from_this<chat_session> {
public:
    chat_session(tcp::socket socket, chat_room& room)
        : socket_(std::move(socket)),
          strand_(asio::make_strand(socket_.get_executor())),
          room_(room),
          name_(make_guest_name()) {}

    void start() {
        room_.join(shared_from_this(), boost_chat::server_line(name_ + " joined from " + remote_name()));
        deliver(boost_chat::server_line("connected as " + name_ + ". Commands: /name <name>, /quit"));
        do_read();
    }

    void deliver(std::string message) {
        asio::post(strand_, [self = shared_from_this(), message = std::move(message)]() mutable {
            if (self->stopped_) {
                return;
            }

            const bool write_in_progress = !self->outbox_.empty();
            self->outbox_.push_back(std::move(message));
            if (!write_in_progress) {
                self->do_write();
            }
        });
    }

private:
    static std::string make_guest_name() {
        const auto uuid = boost::uuids::to_string(boost::uuids::random_generator{}());
        return "guest-" + uuid.substr(0, 8);
    }

    std::string remote_name() const {
        error_code ec;
        const auto endpoint = socket_.remote_endpoint(ec);
        if (ec) {
            return "unknown";
        }

        std::ostringstream out;
        out << endpoint;
        return out.str();
    }

    void do_read() {
        asio::async_read_until(socket_, input_, '\n',
            asio::bind_executor(strand_,
                [self = shared_from_this()](const error_code& ec, std::size_t bytes_transferred) {
                    self->on_read(ec, bytes_transferred);
                }));
    }

    void on_read(const error_code& ec, std::size_t bytes_transferred) {
        if (ec) {
            stop(boost_chat::server_line(name_ + " left"));
            return;
        }

        if (bytes_transferred > boost_chat::max_line_size) {
            deliver(boost_chat::server_line("message too large; closing connection"));
            stop(boost_chat::server_line(name_ + " was disconnected after sending an oversized message"));
            return;
        }

        std::istream stream(&input_);
        std::string line;
        std::getline(stream, line);
        line = boost_chat::trim_wire_line(std::move(line));

        if (line.empty()) {
            do_read();
            return;
        }

        if (line == "/quit") {
            stop(boost_chat::server_line(name_ + " left"));
            return;
        }

        constexpr std::string_view name_command = "/name ";
        if (line.rfind(name_command, 0) == 0) {
            auto new_name = line.substr(name_command.size());
            boost::algorithm::trim(new_name);
            if (!new_name.empty()) {
                const auto old_name = name_;
                name_ = std::move(new_name);
                room_.broadcast(boost_chat::server_line(old_name + " is now " + name_));
            }
            do_read();
            return;
        }

        room_.broadcast(boost_chat::to_wire_line(name_ + ": " + line));
        do_read();
    }

    void do_write() {
        asio::async_write(socket_, asio::buffer(outbox_.front()),
            asio::bind_executor(strand_,
                [self = shared_from_this()](const error_code& ec, std::size_t) {
                    if (ec) {
                        self->stop(boost_chat::server_line(self->name_ + " left"));
                        return;
                    }

                    self->outbox_.pop_front();
                    if (!self->outbox_.empty()) {
                        self->do_write();
                    }
                }));
    }

    void stop(std::string announcement) {
        if (stopped_) {
            return;
        }

        stopped_ = true;
        room_.leave(shared_from_this(), std::move(announcement));

        error_code ignored;
        socket_.shutdown(tcp::socket::shutdown_both, ignored);
        socket_.close(ignored);
    }

    tcp::socket socket_;
    asio::strand<asio::any_io_executor> strand_;
    chat_room& room_;
    asio::streambuf input_;
    std::deque<std::string> outbox_;
    std::string name_;
    bool stopped_ = false;
};

void chat_room::join(std::shared_ptr<chat_session> session, std::string announcement) {
    asio::post(strand_, [this, session = std::move(session), announcement = std::move(announcement)] {
        sessions_.insert(session);
        broadcast_now(announcement);
    });
}

void chat_room::leave(std::shared_ptr<chat_session> session, std::string announcement) {
    asio::post(strand_, [this, session = std::move(session), announcement = std::move(announcement)] {
        if (sessions_.erase(session) > 0 && !announcement.empty()) {
            broadcast_now(announcement);
        }
    });
}

void chat_room::broadcast(std::string message) {
    asio::post(strand_, [this, message = std::move(message)] {
        broadcast_now(message);
    });
}

void chat_room::broadcast_now(const std::string& message) {
    const auto log_line = boost_chat::trim_wire_line(message);
    if (!log_line.empty()) {
        std::cout << log_line << '\n';
    }

    for (const auto& session : sessions_) {
        session->deliver(message);
    }
}

class chat_server {
public:
    chat_server(asio::io_context& io, const tcp::endpoint& endpoint)
        : io_(io), acceptor_(io), room_(io) {
        error_code ec;
        acceptor_.open(endpoint.protocol(), ec);
        throw_on_error(ec, "open acceptor");

        acceptor_.set_option(tcp::acceptor::reuse_address(true), ec);
        throw_on_error(ec, "set reuse_address");

        acceptor_.bind(endpoint, ec);
        throw_on_error(ec, "bind " + endpoint.address().to_string() + ":" + std::to_string(endpoint.port()));

        acceptor_.listen(asio::socket_base::max_listen_connections, ec);
        throw_on_error(ec, "listen");
    }

    void start() {
        do_accept();
    }

    void stop() {
        asio::post(io_, [this] {
            error_code ignored;
            acceptor_.close(ignored);
        });
    }

private:
    static void throw_on_error(const error_code& ec, const std::string& action) {
        if (ec) {
            throw boost::system::system_error(ec, action);
        }
    }

    void do_accept() {
        acceptor_.async_accept(asio::make_strand(io_), [this](const error_code& ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<chat_session>(std::move(socket), room_)->start();
            } else if (acceptor_.is_open()) {
                std::cerr << "accept failed: " << ec.message() << '\n';
            }

            if (acceptor_.is_open()) {
                do_accept();
            }
        });
    }

    asio::io_context& io_;
    tcp::acceptor acceptor_;
    chat_room room_;
};

struct server_options {
    std::string address = "0.0.0.0";
    unsigned short port = 12345;
    int threads = std::max(1u, std::thread::hardware_concurrency());
};

server_options parse_options(int argc, char* argv[]) {
    server_options options;
    po::options_description description("chat_server options");
    description.add_options()
        ("help,h", "show help")
        ("address,a", po::value<std::string>(&options.address)->default_value(options.address), "bind address")
        ("port,p", po::value<unsigned short>(&options.port)->default_value(options.port), "bind TCP port")
        ("threads,t", po::value<int>(&options.threads)->default_value(options.threads), "io_context worker threads");

    po::variables_map variables;
    po::store(po::parse_command_line(argc, argv, description), variables);
    po::notify(variables);

    if (variables.count("help")) {
        std::cout << description << '\n';
        std::exit(0);
    }

    options.threads = std::max(1, options.threads);
    return options;
}

int main(int argc, char* argv[]) {
    try {
        const auto options = parse_options(argc, argv);

        asio::io_context io;
        const tcp::endpoint endpoint(asio::ip::make_address(options.address), options.port);
        chat_server server(io, endpoint);

        asio::signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait([&](const error_code&, int) {
            server.stop();
        });

        server.start();
        std::cout << "chat_server listening on " << endpoint << " with " << options.threads
                  << " io_context thread(s)\n";

        std::vector<std::thread> threads;
        threads.reserve(static_cast<std::size_t>(options.threads));
        for (int i = 0; i < options.threads; ++i) {
            threads.emplace_back([&io] {
                io.run();
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    } catch (const std::exception& ex) {
        std::cerr << "chat_server: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
