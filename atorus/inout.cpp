#include <iostream>
#include <vector>
#include <string>

void hello() 
{
    std::cout << "Enter your name\n";
    std::string str;
    std::cin >> str;
    std::cout << "Hello, " << str << "!\n";
}

void hello_line() 
{
    std::cout << "Enter name: \n";
    std::string str;
    std::getline(std::cin, str);
    std::cout << "System display " << str << "\n"; 
}

std::vector<int> read_ints(std::istream& is) 
{
    std::cout << "Enter values of type integers: ";
    std::vector<int> res;
    for (int i; is>>i;) 
    {
        res.push_back(i);
    } 
    return res;
}

std::vector<int> read_ints(std::istream& is, const std::string& terminator) 
{   
    std::vector<int> res;
    for (int i; is>>i;) {
        res.push_back(i);
    }
    if (is.eof()) 
        return res;
    if (is.fail()) {
        is.clear();
        is.unget();
        std::string s;
        if (std::cin >> s && s==terminator) 
            return res;
        std::cin.setstate(std::ios_base::failbit);
    }
    return res;
}

struct Entry {
    std::string name;
    int number;
};

std::ostream& operator<<(std::ostream& os, const Entry& e) 
{
    return os << "{\"" << e.name <<  "\", " << e.number << "}";
}

std::istream& operator>>(std::istream& is, Entry& e) 
{
    char c, c2;
    if (is >> c && c=='{' && is>>c2 && c2=='"') {
        std::string name;
        while (is.get(c) && c!='"') 
            name+=c;
        
        if (is>>c && c==',') {
            int number = 0;
            if (is>>number>>c && c=='}') {
                e = {name,number};
                return is;
            }
        }
    }
    is.setstate(std::ios_base::failbit);
    return is;   
}


int main() 
{
    // hello();
    hello_line();

    auto values = read_ints(std::cin);  
    std::cout << "Read values in system: ";
    for (int x : values) 
    {
        std::cout << x << " ";
    }
    std::cout << '\n';

    auto v = read_ints(std::cin, "stop");
    return 0;
}
