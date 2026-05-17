#pragma once

#include <string>

namespace code {

class X {
private:
    int m;  
public:
    X(int i=0) : m{i} {}
    
    int mf(int i) {
        int old = m;
        m = i;
        return old;
    }
};

template<typename T>
class Tree {
    using value_type = T;
    enum Policy { rb, splay, treeps };
    class Node {
        Node* right;
        Node* left;
        value_type value;
    public:
        void f(Tree*);
    };
    Node* top;
public:
    void g(const T&);
};

template<typename T>
void Tree<T>::Node::f(Tree<T>* p) {
    top = right;
    p->top = right;
    value_type v = left->value_type;
}

namespace Relativity {

enum class Month { an=1, feb, mar, apr, may, jun, jul, aug, sep, oct, nov, dec };

class Date {
public:
    class Bad_date{};
    explicit Date(int dd = {}, Month mm = {}, int yy ={});

    int day() const;
    Month month() const;
    int year() const;

    std::string string_rep() const;
    void char_rep(char s[], int mx) const; // C-style string rep

    Date& add_year(int n); 
    Date& add_month(int n);
    Date& add_day(int n);

private:
    bool is_valid();
    int d, m, y;
};

bool is_date(int d, Month m, int y);

ostream& operator<<(ostream& os, const Date& d);
istream& operator>>(istream& is, Date& d);

} // namespace Relativity

} // namespace code