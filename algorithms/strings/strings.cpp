#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <stack>
#include <unordered_map>
using namespace std;

bool is_interleave(const string& s1, const string& s2, const string& s3) {
    // time O(m*n)
    // spcae O(m*n)
    int m = s1.size(), n = s2.size();
    if (m+n!=s3.size()) 
        return false;
    vector<vector<bool>> dp(m+1,vector<bool>(n+1));
    dp[0][0] = true;
    for (int j=1; j<=n;j++) {
        dp[0][j] = (s2[j-1] == s3[j-1]) && dp[0][j-1];
    }
    for (int i=1; i<=m; i++) {
        dp[i][0] = (s1[i-1] == s3[i-1]) && dp[i-1][0];
    }

    for (int i=1; i<=m; i++) {
        for (int j=1;j<=n;j++) {
            int k=i+j-1;
            dp[i][j] = ((s1[i-1] == s3[k]) && dp[i-1][j]) || ((s2[j-1] == s3[k]) && dp[i][j-1]);
        }
    }
    return dp[m][n];
}

auto group_anagrams(const vector<string>& ana) -> vector<vector<string>> {
    // time O((n*k)logk)
    // space O(n*k)
    unordered_map<string, vector<string>> gg;
    for (const auto& s : ana) {
        string tmp = s;
        sort(tmp.begin(), tmp.end());
        gg[tmp].push_back(s);
    }
    vector<vector<string>> ans;
    for (const auto& [_, v] : gg) {
        ans.push_back(v);
    }
    return ans;
}

auto remove_k_chars(int k, const string& s) -> string {
    // time O(n)
    // space O(n)
    int n = s.size();
    if (k==1) return "";
    stack<pair<int,int>> st;
    for (int i=0;i<n;i++) {
        if (st.empty()) {
            st.push({s[i], 1});
        } else {
            if (st.top().first == s[i]) {
                auto [ch, cnt] = st.top();
                st.pop();
                cnt++;
                if (cnt < k) st.push({s[i], cnt});
            } else {
                st.push({s[i], 1});
            }
        }
    }

    string res="";
    while (!st.empty()) {
        if (st.top().second > 1) {
            auto [ch, cnt] = st.top();
            while (cnt--) 
                res += ch;
        } else {
            res += st.top().first;
        }
        st.pop();
    }
    reverse(res.begin(), res.end());
    return res;
}

enum Problem {
    INTERLEAVE=0,  
    GROUP_ANAGRAMS,
    REMOVE_K_CONSECUTIVE_INDETICAL_CHARS,
    KMP,
};

int main() {
    ios::sync_with_stdio(0); cin.tie(0);

    Problem problem = Problem::REMOVE_K_CONSECUTIVE_INDETICAL_CHARS;

    switch (problem) {
        case INTERLEAVE: {
            string s1 = "AAB", s2 = "AAC", s3 =  "AAAABC";
            cout << is_interleave(s1,s2,s3) << '\n';
            break;
        }
        case GROUP_ANAGRAMS: {
            vector<string> arr{"act", "god", "cat", "dog", "tac"};
            vector<vector<string>> res = group_anagrams(arr);
            for (const auto& f : res) {
                for (const auto& s : f) {
                    cout << s << " ";
                }
                cout << '\n';
            }   
            break;
        }
        case REMOVE_K_CONSECUTIVE_INDETICAL_CHARS: {
            string s = "geeksforgeeks";
            int k = 2;
            cout << remove_k_chars(k, s) << "\n";
            break;
        }
        case KMP: {
            
            break;
        }
        default: 
            break;
    }

    return 0;   
}