#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <stack>
#include <unordered_map>
using namespace std;

enum Problem {
    INTERLEAVE=0,  
    GROUP_ANAGRAMS,
    REMOVE_K_CONSECUTIVE_INDETICAL_CHARS,
    KMP,
    SHORTEST_COMMON_SUBSEQUENCE,
    LONGEST_SUBSTRING_REARRANGED_PALINDROME,
};

// All substrings of a given String
auto find_substrings(const string& s) -> vector<string> {
    // time O(n^3)
    // space O(n^3)
    int n=s.size();
    vector<string> res;
    for (int i=0;i<n;i++) {
        for (int j=i;j<n;j++) {
            res.push_back(s.substr(i, j-i+1));
        }
    }
    return res;
}

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

//Given two strings txt and pat, the task is to return all indices of occurrences of pat within txt.
auto search_for_pat(const string& pat, const string& txt) -> vector<int> {
    // time O(n+m)
    // space O(m)
    auto LPS = [&](const string& s) -> vector<int> {
        int m=s.size();
        vector<int> lps(m);
        int len=0;
        int i=1;
        while (i<m) {
            if (s[i]==s[len]) {
                len++;
                lps[i]=len;
                i++;
            } else {
                if (len!=0) {
                    len=lps[len-1];
                } else {
                    lps[i]=0;
                    i++;
                }
            }
        }
        return lps;
    };

    int n=txt.size();
    int m=pat.size();
    vector<int> lps = LPS(pat);
    vector<int> res;

    int i=0, j=0;
    while (i<n) {
        if (txt[i]==pat[j]) {
            i++; j++;
            if (j==m) {
                res.push_back(i-j);
                j=lps[j-1];
            }
        } else {
            if (j!=0) {
                j=lps[j-1];
            } else {
                i++;
            }
        }
    }
    return res;
}

// Given two strings s1 and s2, Find the length of the shortest string that has both s1 and s2 as subsequences.
// A subsequence of a string is a sequence that can be derived from the string by deleting zero or more characters 
// without changing the relative order of the remaining characters.
auto shortest_common_subseq(const string& s, const string& t) -> int {
    // time O(m*n)
    // space O(m*n)
    int m=s.size(), n=t.size();
    vector<vector<int>> dp(m+1,vector<int>(n+1));
    for (int i=0;i<=m;i++) {
        dp[i][0]=i;
    }
    for (int j=0;j<=n;j++) {
        dp[0][j]=j;
    }
    for (int i=1;i<=m;i++) {
        for (int j=1;j<=n;j++) {
            if (s[i-1]==t[j-1]) {
                dp[i][j]=1+dp[i-1][j-1];
            } else {
                dp[i][j]= 1 + min(dp[i-1][j],dp[i][j-1]);
            }
        }
    }
    return dp[m][n];
}

// Given a string S of length N which only contains lowercase alphabets. Find the length of the longest substring 
// of S such that the characters in it can be rearranged to form a palindrome. 
int longest_substring(string s, int n) {
    // time O(N*26)
    // space O(N*26)
    // Stores: prefix mask -> earliest index where this mask was seen
    unordered_map<int, int> index;

    // Best answer found so far
    int answer = 0;

    // Current prefix parity mask
    int mask = 0;

    /*
        Before processing any character, the prefix is empty.
        Its parity mask is 0 because every count is even (all zero).

        We pretend this happened at index -1.
        This is important so that substrings starting at index 0 are handled correctly.

    */
    index[0] = -1;

    for (int i = 0; i < n; i++) {
        // Convert current character into a bit position [0..25]
        int temp = s[i] - 'a';
        /*
            Toggle the bit for s[i].

            If this character had appeared an even number of times before,
            it now appears odd number of times.
            If it had appeared odd number of times before,
            it now becomes even.
        */
        mask ^= (1 << temp);
        /*
            --------------------------------------------------------
            Case 1: substring has ALL even counts
            --------------------------------------------------------

            If we have seen this exact same mask before at some earlier index p,
            then the substring (p+1 .. i) has parity mask 0.

            Why?
              currentPrefixMask XOR oldPrefixMask = 0
              because they are the same

            That means every character appears an even number of times
            in this substring, so it can be rearranged into a palindrome.
        */
        if (index.count(mask)) {
            answer = max(answer, i - index[mask]);
        } else {
            /*
                Store only the earliest occurrence of this mask.

                We do NOT update it later, because an earlier position gives
                a longer substring when matched with a future index.
            */
            index[mask] = i;
        }
        /*
            --------------------------------------------------------
            Case 2: substring has EXACTLY ONE odd count
            --------------------------------------------------------

            A substring can also form a palindrome if exactly one character
            occurs odd number of times.

            To check this, we try flipping each of the 26 bits in the current mask.

            If mask2 differs from mask by exactly one bit, and mask2 was seen before,
            then:
                currentMask XOR oldMask = one-bit mask
            which means the substring has exactly one odd-count character.
        */
        for (int j = 0; j < 26; j++) {
            // Flip j-th bit of current mask
            int mask2 = mask ^ (1 << j);

            // If such a prefix mask appeared before, we found a valid substring
            if (index.count(mask2)) {
                answer = max(answer, i - index[mask2]);
            }
        }
    }

    return answer;
}

int main() {
    ios::sync_with_stdio(0); cin.tie(0);

    Problem problem = Problem::LONGEST_SUBSTRING_REARRANGED_PALINDROME;

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
            string txt = "aabaacaadaabaaba";
            string pat = "aaba";

            vector<int> res = search_for_pat(pat, txt);
            for (int i = 0; i < res.size(); i++)
                cout << res[i] << " ";
            break;
        }
        case SHORTEST_COMMON_SUBSEQUENCE: {
            string s1 = "AGGTAB";
            string s2 = "GXTXAYB";
            int res = shortest_common_subseq(s1, s2);
            cout << res << endl;
        }
        case LONGEST_SUBSTRING_REARRANGED_PALINDROME: {
            string s = "adbabd";
            cout << longest_substring(s, s.size()) << '\n';
            break;
        }
        default: 
            break;
    }

    return 0;   
}