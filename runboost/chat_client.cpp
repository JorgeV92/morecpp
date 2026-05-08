#include "chat_protocol.hpp"

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace asio = boost::asio;
namespace po = boost::program_options;
using boost::asio::ip::tcp;
using error_code = boost::system::error_code;

class chat_client : public std::enable_shared_from_this<chat_client> {
public:
    chat_client(asio::io_context& io, std::string name)
        : io_(io),
          resolver_(io),
          socket_(io),
          strand_(asio::make_strand(io)),
          name_(std::move(name)) {}

    void start(const std::string& host, const std::string& port) {
        resolver_.async_resolve(host, port,
            asio::bind_executor(strand_,
                [self = shared_from_this()](const error_code& ec, tcp::resolver::results_type endpoints) {
                    if (ec) {
                        std::cerr << "resolve failed: " << ec.message() << '\n';
                        self->io_.stop();
                        return;
                    }

                    asio::async_connect(self->socket_, endpoints,
                        asio::bind_executor(self->strand_,
                            [self](const error_code& connect_ec, const tcp::endpoint& endpoint) {
                                self->on_connect(connect_ec, endpoint);
                            }));
                }));
    }

    void write(std::string line) {
        asio::post(strand_, [self = shared_from_this(), message = boost_chat::to_wire_line(std::move(line))]() mutable {
            self->enqueue_write(std::move(message));
        });
    }

    void write_and_close(std::string line) {
        asio::post(strand_, [self = shared_from_this(), message = boost_chat::to_wire_line(std::move(line))]() mutable {
            self->close_after_write_ = true;
            self->enqueue_write(std::move(message));
        });
    }

    void close() {
        asio::post(strand_, [self = shared_from_this()] {
            self->close_now();
        });
    }

private:
    void on_connect(const error_code& ec, const tcp::endpoint& endpoint) {
        if (ec) {
            std::cerr << "connect failed: " << ec.message() << '\n';
            io_.stop();
            return;
        }

        connected_ = true;
        std::cout << "connected to " << endpoint << '\n';
        if (!name_.empty()) {
            outbox_.push_front(boost_chat::to_wire_line("/name " + name_));
        }
        do_read();
        if (!outbox_.empty()) {
            do_write();
        }
    }

    void enqueue_write(std::string message) {
        if (closed_) {
            return;
        }

        const bool write_in_progress = !outbox_.empty();
        outbox_.push_back(std::move(message));
        if (connected_ && !write_in_progress) {
            do_write();
        }
    }

    void do_read() {
        asio::async_read_until(socket_, input_, '\n',
            asio::bind_executor(strand_,
                [self = shared_from_this()](const error_code& ec, std::size_t) {
                    if (ec) {
                        if (ec != asio::error::operation_aborted) {
                            std::cerr << "read stopped: " << ec.message() << '\n';
                        }
                        self->close_now();
                        return;
                    }

                    std::istream stream(&self->input_);
                    std::string line;
                    std::getline(stream, line);
                    line = boost_chat::trim_wire_line(std::move(line));
                    std::cout << line << '\n';

                    self->do_read();
                }));
    }

    void do_write() {
        asio::async_write(socket_, asio::buffer(outbox_.front()),
            asio::bind_executor(strand_,
                [self = shared_from_this()](const error_code& ec, std::size_t) {
                    if (ec) {
                        if (ec != asio::error::operation_aborted) {
                            std::cerr << "write failed: " << ec.message() << '\n';
                        }
                        self->close_now();
                        return;
                    }

                    self->outbox_.pop_front();
                    if (!self->outbox_.empty()) {
                        self->do_write();
                    } else if (self->close_after_write_) {
                        self->close_now();
                    }
                }));
    }

    void close_now() {
        if (closed_) {
            return;
        }

        closed_ = true;
        error_code ignored;
        socket_.shutdown(tcp::socket::shutdown_both, ignored);
        socket_.close(ignored);
        io_.stop();
    }

    asio::io_context& io_;
    tcp::resolver resolver_;
    tcp::socket socket_;
    asio::strand<asio::io_context::executor_type> strand_;
    asio::streambuf input_;
    std::deque<std::string> outbox_;
    std::string name_;
    bool closed_ = false;
    bool connected_ = false;
    bool close_after_write_ = false;
};

struct client_options {
    std::string host = "127.0.0.1";
    std::string port = "12345";
    std::string name = asio::ip::host_name();
};

client_options parse_options(int argc, char* argv[]) {
    client_options options;
    po::options_description description("chat_client options");
    description.add_options()
        ("help,h", "show help")
        ("host,H", po::value<std::string>(&options.host)->default_value(options.host), "server host")
        ("port,p", po::value<std::string>(&options.port)->default_value(options.port), "server TCP port")
        ("name,n", po::value<std::string>(&options.name)->default_value(options.name), "chat display name");

    po::variables_map variables;
    po::store(po::parse_command_line(argc, argv, description), variables);
    po::notify(variables);

    if (variables.count("help")) {
        std::cout << description << '\n';
        std::exit(0);
    }

    return options;
}

int main(int argc, char* argv[]) {
    try {
        const auto options = parse_options(argc, argv);

        asio::io_context io;
        const auto client = std::make_shared<chat_client>(io, options.name);

        asio::signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait([client](const error_code&, int) {
            client->close();
        });

        client->start(options.host, options.port);

        std::atomic_bool input_done = false;
        std::thread input_thread([client, &input_done] {
            std::string line;
            bool requested_quit = false;
            while (std::getline(std::cin, line)) {
                if (line == "/quit") {
                    client->write_and_close(line);
                    requested_quit = true;
                    break;
                }
                client->write(line);
            }
            if (!requested_quit) {
                client->close();
            }
            input_done = true;
        });

        io.run();

        if (input_thread.joinable()) {
            if (input_done) {
                input_thread.join();
            } else {
                input_thread.detach();
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "chat_client: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
