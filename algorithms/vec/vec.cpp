#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <numeric>

void use() {
    std::cout << sizeof(char) << '\n';
    std::cout << sizeof(int) << '\n';
    std::cout << sizeof(int64_t) << '\n';
    std::vector<int> v(1000);
    std::cout << sizeof(v) << '\n';
}

void call17() {
    int* ptr = new int[10]{};
    for (int i =0 ;i < 10; i++) {
        std::cout << ptr[i] << " ";
    }
    std::cout << '\n';
    delete[] ptr;
}   

void to_lower(char* s) {
    if (s == nullptr) return;
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] = s[i] + ('a' - 'A');
        }
    }
}

char* strdup(const char* s) {
    if (s == nullptr) return nullptr;
    int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    char* cpy = new char[len+1];
    for (int i = 0; i < len; i++) {
        cpy[i] = s[i];
    }
    cpy[len] = '\0';
    return cpy;
}

const char* findx(const char* s, const char* x) {
    if (s == nullptr || x == nullptr) return nullptr;
    if (x[0] == '\0') {
        return s;
    }
    for (int i =0; s[i] != '\0'; i++) {
        int j = 0;
        while(s[i+j] != '\0' && x[j] != '\0' && s[i+j] == x[j]) j++;
        if (x[j] == '\0') {
            return &s[i];
        }
    }
    return nullptr;
}

void read_chars() {
    int cap = 8;
    int sz = 0;
    char* buffer = new char[cap];
    char ch;
    while (std::cin.get(ch)) {
        if (ch == '!') {
            break;
        }
        if (sz == cap) {
            int new_cap = cap*2;
            char* more = new char[new_cap];
            for (int i = 0; i < sz; i++) {
                more[i] = buffer[i];
            }
            delete[] buffer;
            buffer = more;
            cap = new_cap;
        }
        buffer[sz] = ch;
        sz++;
    }
}

int main() {

    use();
    call17();

    auto call17_2 = []() {
        char t[] = "Hello, World!";
        to_lower(t);
        std::cout << t << '\n';
    };

    auto call17_3 = []() {
        const char* org = "Hello, World!";
        char* c = strdup(org);
        std::cout << c << '\n';
        delete[] c;
    };

    call17_2();
    call17_3();

    return 0;
}