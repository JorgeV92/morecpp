#pragma once

#include <iostream>
#include <string>


template<typename T>
struct __link {
    T value;
    __link* prev;
    __link* succ;
    __link(const T& v, __link* p = nullptr, __link* s = nullptr) 
        : value(v), prev(p), succ(s) {}

    void __print_list_(__link<T>* node) {
        auto head = node;
        while (head) {
            std::cout << head->value << " ";
            head = head->succ;
        }
        std::cout << "\n";
    }

    __link<T>* __insert_(__link<T>* p, __link<T>* n) { // insert n before p
        if (n == nullptr) return p; // nothing to insert
        if (p == nullptr) return n; // n is the first node
        n->succ = p;
        if (p->prev)
            p->prev->succ = n;
        n->prev = p->prev;
        p->prev = n;
        return n;
    }

    __link<T>* __add_(__link<T>* p, __link<T>* n) { // insert after p; return n
        if (p == nullptr) return n;
        if (n == nullptr) return p;
        n->prev = p;
        if (p->succ)
            p->succ->prev = n;
        n->succ = p->succ;
        p->succ = n;
        return n;
    }

    __link<T>* __erase_(__link<T>* p) { // remove *p from list; return p's succesor
        if (p == nullptr) return nullptr;
        if (p->succ) p->succ-prev = p->prev;
        if (p->prev) p->prev->succ = p->succ;
        return p->succ;
    }

    __link<T>* __find_(__link<T>* p, const T& s) {
        while (p) {
            if (p->value == s) return p;
            p = p->succ;
        }
        return nullptr;
    }

    __link<T>* __advance_(__link<T>* p, int n) {
        if (p == nullptr) return nullptr;
        if (n > 0) {
            while (n--) {
                if (p->succ == nullptr) return nullptr;
                p = p->succ;
            }
        } else if (n < 0) {
            while (n++) {
                if (p->prev == nullptr) return nullptr;
                p = p->prev;
            }
        }
        return p;
    }
};


auto __example() -> void {
    __link<std::string>* norse_gods = new __link<std::string>{"Thor", nullptr, nullptr};
    norse_gods = new __link<std::string>{"Odin", nullptr, norse_gods};
    norse_gods->succ->prev = norse_gods;
    norse_gods = new __link<std::string>{"Freia", nullptr, norse_gods};
    norse_gods->succ->prev = norse_gods;
    norse_gods->__print_list_(norse_gods);
}

auto __exmaple_insert() -> void {
    __link<std::string>* norse_gods = new __link<std::string>{"Thor"};
    norse_gods = norse_gods->__insert_(norse_gods, new __link<std::string>{"Odin"});
    norse_gods = norse_gods->__insert_(norse_gods, new __link<std::string>{"Freia"});
    norse_gods->__print_list_(norse_gods);
}

auto __example_integers() {
    __link<int>* v = new __link<int>(4);
    v = v->__insert_(v, new __link<int>(6));
    v = v->__insert_(v, new __link<int>(7));
    for (int i = 1; i <= 10; i++) {
        v = v->__insert_(v, new __link<int>(i));
    }
    v->__print_list_(v);
    v = v->__advance_(v, 4);
    std::cout << v->value << "\n";
}

