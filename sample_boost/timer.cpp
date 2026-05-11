#include <iostream>
#include <boost/asio.hpp>
#include <functional>
#include <thread>

class printer_multi {
public:
    printer_multi(boost::asio::io_context& io) 
        : strand_(boost::asio::make_strand(io)),
          timer1_(io, boost::asio::chrono::seconds(1)),
          timer2_(io, boost::asio::chrono::seconds(1)),
          count_(0) {
        
        timer1_.async_wait(boost::asio::bind_executor(strand_,
                            std::bind(&printer_multi::print1, this)));
        timer2_.async_wait(boost::asio::bind_executor(strand_,
                            std::bind(&printer_multi::print2, this)));
    }

    ~printer_multi() {
        std::cout << "Final count is " << count_ << std::endl;
    }

    void print1() {
        if (count_ < 10) {
            std::cout << "Tiimer 1: " << count_ << std::endl;
            count_++;
            timer1_.expires_at(timer1_.expiry() + boost::asio::chrono::seconds(1));
            timer1_.async_wait(boost::asio::bind_executor(strand_,
                                std::bind(&printer_multi::print1, this)));
        }
    }

    void print2() {
        if (count_ < 10) {
            std::cout << "Timer 2: " << count_ << std::endl;
            count_++;
            timer2_.expires_at(timer2_.expiry() + boost::asio::chrono::seconds(1));
            timer2_.async_wait(boost::asio::bind_executor(strand_, 
                                std::bind(&printer_multi::print2, this)));
        }
    }

private:
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer timer1_;
    boost::asio::steady_timer timer2_;
    int count_;
};

class printer {
public:
    printer(boost::asio::io_context& io) 
        : timer_(io, boost::asio::chrono::seconds(1)), count_(0) {
        timer_.async_wait(std::bind(&printer::print, this));
    }
    
    ~printer() {
        std::cout << "Final count is " << count_ << std::endl;
    }

    void print() {
        if (count_ < 5) {
            std::cout << count_ << std::endl;
            ++count_;
            timer_.expires_at(timer_.expiry() + boost::asio::chrono::seconds(1));
            timer_.async_wait(std::bind(&printer::print, this));
        }
    }

private:
    boost::asio::steady_timer timer_;
    int count_;
};

void print(const boost::system::error_code&) {
    std::cout << "Hello, world!" << std::endl;
}

void print_args(const boost::system::error_code&, 
                boost::asio::steady_timer* t, int* count) {
    if (*count < 5) {
        std::cout << *count << std::endl;
        ++(*count);
        t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));
        t->async_wait(std::bind(print_args, boost::asio::placeholders::error, t, count));
    }
}

int main() {

    auto sync = []() -> void {
        boost::asio::io_context io;
        boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));
        t.wait();
        std::cout << "Hello, world!" << std::endl;
    };

    auto async = []() -> void {
        boost::asio::io_context io;
        boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));
        t.async_wait(&print);
        io.run();
    };

    auto async_2 = []() -> void {
        boost::asio::io_context io;
        int count = 0;
        boost::asio::steady_timer t(io, boost::asio::chrono::seconds(1));
        t.async_wait(std::bind(print_args, boost::asio::placeholders::error, &t, &count));
        io.run();
        std::cout << "Final count is " << count << '\n';
    };  

    auto async_class = []() {
        boost::asio::io_context io;
        printer p(io);
        io.run();
    };
    
    auto async_multi_thread = []() -> void {
        boost::asio::io_context io;
        printer_multi pm(io);
        std::thread t([&]{ io.run(); });
        io.run();
        t.join();
    };

    // async();
    // async_2();
    // async_class();
    async_multi_thread();

    return 0;
}

/*
clang++ -std=c++17 -Wall -Wextra -pthread -I/opt/homebrew/opt/boost/include -L/opt/homebrew/opt/boost/lib timer.cpp -o timmer -lboost_system 
*/
