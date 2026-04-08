#include <iostream>
#include <vector>
#include <string>
#include <string_view>
using namespace std;

enum DP_PROBELMS {
    MINIMIZE_TO_REACH_K=0,
    TRANSFORM_STRINGS,
};

// Given a positive integer K, the task is to find the minimum number of operations of the following two types, required to change 0 to K.
auto min_operations_reach_k(int k) -> int {
    // time O(k)
    // space O(k)
    vector<int> dp(k+1);
    for (int i=1;i<=k;i++) {
        dp[i]=dp[i-1]+1;
        if (i%2==0) {
            dp[i]=min(dp[i],dp[i/2]+1);
        }
    }
    return dp[k];
}

// Given two strings s1 and s2. The task is to remove/delete and insert the minimum number of 
// characters from s1 to transform it into s2. It could be possible that the same character 
// needs to be removed/deleted from one point of s1 and inserted at another point.
auto min_operations_strings(const string& s, const string& t) -> int {
    // time O(N^2)
    // space O(N^2)
    auto lcs = [&](string_view sv1, string_view sv2) -> int {
        int m=sv1.size(), n=sv2.size();
        vector<vector<int>> dp(m+1,vector<int>(n+1));
        for (int i=1;i<=m;i++) {
            for (int j=1;j<=n;j++) {
                if (sv1[i-1]==sv2[j-1]) {
                    dp[i][j] = 1 + dp[i-1][j-1];
                } else {
                    dp[i][j] = max(dp[i-1][j],dp[i][j-1]);
                }
            }
        }
        return dp[m][n];
    };

    int len=lcs(s,t);
    int del=s.size()-len;
    int in=t.size()-len;
    return del+in;
}

int main() {

    DP_PROBELMS problem = DP_PROBELMS::MINIMIZE_TO_REACH_K;

    switch (problem) {
        case TRANSFORM_STRINGS: {
            string s1 = "AGGTAB";
            string s2 = "GXTXAYB";
            int res = min_operations_strings(s1, s2);
            cout<<res;
            break;
        }
        case MINIMIZE_TO_REACH_K: {
            int k=12;
            cout<<min_operations_reach_k(k)<<'\n';
            break;
        }
        default:
            break;

    }
    return 0;
}