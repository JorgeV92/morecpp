#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <string>
#include <sstream>

using namespace std;

int main() {
    ifstream in("words.txt");
    if (!in) {
        cerr << "Could not open words.txt\n";
        exit(1);
    }
    vector<pair<string, string>> words;
    string word;
    while (getline(in, word)) {
        transform(word.begin(), word.end(), word.begin(),
                  [](unsigned char c) { return tolower(c); });

        string sig = word;
        sort(sig.begin(), sig.end());

        words.push_back({sig, word});
    }
    sort(words.begin(), words.end());
    ofstream out("anagrams.txt");
    if (!out) {
        cerr << "Could not open anagrams.txt\n";
        exit(1);
    }
    for (auto& [sig, word] : words) {
        out << sig << ' ' << word << '\n';
    }
    out.close();
    ifstream in2("anagrams.txt");
    if (!in2) {
        cerr << "Could not open anagrams.txt\n";
        exit(1);
    }
    string sig, org;
    string old;
    bool first = true;
    while (in2 >> sig >> org) {
        if (!first && sig != old) {
            cout << '\n';
        }
        cout << org << " ";
        old = sig;
        first = false;
    }
    cout << '\n';

    return 0;
}