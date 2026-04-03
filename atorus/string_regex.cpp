#include <iostream>
#include <string>
#include <regex>
#include <fstream>
using namespace std;

string compose(const string& name, const string& domain) {
    return name + '@' + domain;
} 

void m2(string& s1, string& s2) {
    s1 = s1 + '\n';
    s2 += '\n';
}

void search() {
    regex pat{R"(\w{2}\s*\d{5}(-\d{4})?)"};
    int lineno = 0;
    for (string line; getline(cin, line);) {
        ++lineno;
        smatch matches;
        if (regex_search(line, matches, pat)) 
            cout << lineno << ": " << matches[0] << "\n";
    } 
}

void search_file() {
    ifstream in("input.txt");
    if (!in) 
        cerr << "no file\n";

    regex pat{R"(\w{2}\s*\d{5}(-\d{4})?)"};
    int lineno = 0;
    for (string line; getline(in,line);) {
        ++lineno;
        smatch matches;
        if (regex_search(line, matches, pat)) {
            cout << lineno << ": " << matches[0] << '\n';
            if (1 < matches.size() && matches[1].matched) 
                cout << "\t: " << matches[1] << "\n";
        }
    }
}

int main() {
    auto addr = compose("dmr", "bell-labs.com");
    cout << addr << '\n';
    cout << "Enter for search: \n";
    // search();
    search_file();
    return 0;
}