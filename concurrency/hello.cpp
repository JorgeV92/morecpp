#include <iostream>
#include <vector>
#include <thread>

class background_task {
public:
    void operator()() const {
        do_something();
        do_something_else();
    }
    void do_something() const {
        std::cout << "Do something\n";
    }
    void do_something_else() const {
        std::cout << "Doing something else\n";    
    }
};

void hello() {
    std::cout << "Hello Concurrent World\n";
}

struct func {
    int& i;
    func(int& _i) : i(_i) {}

    void operator()() {
        for (unsigned j = 0; j < 1000; j++) {
            do_loop(i);
        }
    }
    void do_loop(int& i) {
        i++;
        std::cout << i << " ";
    }
};

class thread_guard {
public:
    thread_guard(std::thread t_) : t(std::move(t_)) {}

    ~thread_guard() {
        if (t.joinable()) {
            t.join();
        }
    }
    thread_guard(thread_guard const&)=delete;
    thread_guard& operator=(thread_guard const&)=delete;


private:
    std::thread t;
};

void ff() {
    int state = 0;
    thread_guard g{std::thread(func(state))};
} 

int main() {
    ff();
    std::thread t(hello);
    t.join();
    background_task f;
    std::thread my_thread(f);
    my_thread.join();
    int state = 0;
    func my_func(state);
    std::thread my_thread_func(my_func);
    my_thread_func.join();
    return 0;
}