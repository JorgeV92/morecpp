#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <cstdint>
#include <queue>
#include <numeric>
#include <algorithm>
using namespace std;

enum DP_PROBELMS {
    // EASY
    MINIMIZE_TO_REACH_K=0,
    TRANSFORM_STRINGS,
    // MEDIUM 
    KNAPSACK01,
    NUMBER_BLAST,
    PERFECR_SQAURE,
    LONGEST_PALINDROMIC_SUBSTRING,
    // HARD
    PARTITION_SET,
    MAXIMUM_PROFIT_JOB_SCHEDULING,
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

auto knapsack_2d(int W, vector<int>& v, vector<int>& wt) -> int {
    // time O(n*w)
    // space O(n*w)
    int n=wt.size();
    vector<vector<int>> dp(n+1,vector<int>(W+1));
    for (int i=0;i<=n;i++) {
        for (int j=0;j<=W;j++) {
            if (i==0||j==0) {
                dp[i][j]=0; 
            } else {
                int take=0;
                if (wt[i-1]<=j) 
                    take=dp[i-1][j-wt[i-1]]+v[i-1];
                int pass=dp[i-1][j];
                dp[i][j]=max(take,pass);
            }
        }
    }
    return dp[n][W];
}

auto knapsack_opt(int W, vector<int>& v, vector<int>& wt) -> int {
    // time O(N*W)
    // space O(W)
    vector<int> dp(W+1);
    for (int i=1;i<=(int)wt.size();i++) {
        for (int j=W; j >=wt[i-1];j--) {
            dp[j]=max(dp[j],dp[j-wt[i-1]]+v[i-1]);
        }
    }
    return dp[W];
}


auto number_blast_prefix(const vector<int64_t>& A) -> int64_t {    
    //time O(N^2)
    // space O(N)
    int n=A.size();
    vector<int64_t> dp(n+1);
    for (int i=1;i<=n;i++) {
        dp[i]=dp[i-1];
        for (int j=1;j<i;j++) {
            dp[i] = max(dp[i], dp[j-1]+A[j-1]*A[i-1]);
        }
    }
    return dp[n];
}

auto number_blast_suffix(const vector<int64_t>& A) -> int64_t {
    //time O(N^2)
    // space O(N)
    int n=A.size();
    vector<int64_t> dp(n+1);
    for (int i=n-1;i>=0;i--) {
        int64_t best=0;
        for (int j=i+1;j<n;j++) {
            best=max(best,dp[j+1]+A[i]*A[j]);
        }
        if (i+1<n) {
            for (int j=i+2;j<n;j++) {
                best=max(best,A[i+1]*A[j]+dp[j+1]);
            }
        }
        dp[i]=best;
    }
    return dp[0];
}

// Given a positive integer n, find the minimum number of perfect squares that sum up to n. We can use each square any number of times.
auto min_square(int n) -> int {
    // time O(n*sqrt(n))
    // space O(n)
    // dp[i] = minimum number of perfect squares needed to make sum i
    vector<int> dp(n+1);
    dp[0]=0;
    dp[1]=1;
    for (int i=2;i<=n;i++) {
        dp[i]=i;
        for (int x=1;x*x<=i;x++) {
            dp[i]=min(dp[i],dp[i-x*x]+1);
        }
    }
    return dp[n];
}

auto min_square_bfs(int n) -> int {
    // time O(n*sqrt(n))
    // space O(n)
    queue<pair<int,int>> q;
    vector<bool> vis(n+1);
    q.push({n,0});
    vis[n]=true;
    while (!q.empty()) {
        auto p = q.front();
        q.pop();
        int v=p.first;
        int step=p.second;
        if (v==0)
            return step;
        for (int x=1;x*x<=v;x++) {
            int y=v-(x*x);
            if (!vis[y]) {
                vis[y]=true;
                q.push({y,step+1});
            }
         }
    }
    return -1;
}

// Given a string s, find the longest substring which is a palindrome. If there are multiple answers, then find the first appearing substring.
auto longest_pal_subs_dp(const string& s) -> string {    
    // time O(N^2)
    // space O(N^2)
    int n=s.size();
    vector<vector<bool>> dp(n,vector<bool>(n));
    pair<int,int> p{0,1};
    for (int i=0;i<n;i++) {
        dp[i][i]=true;
    }
    for (int i=0;i<n-1;i++) {
        if (s[i]==s[i+1]) {
            dp[i][i+1]=true;
            if(p.second==1) {
                p.first=i;
                p.second=2;
            }
        }
    }
    for (int len=3;len<=n;len++) {
        for (int i=0;i<=n-len;i++) {
            int j=i+len-1;
            if (s[i]==s[j]&&dp[i+1][j-1]) {
                dp[i][j]=true;
                if (len>p.second) {
                    p.first=i;
                    p.second=len;
                }
            }
        }
    }
    return s.substr(p.first,p.second);
}

auto longest_pal_subs_center(const string& s) -> string {
    // time O(N^2)
    // space O(1)
    int n=s.size();
    pair<int,int>p{0,1};
    for (int i=0;i<n;i++) {
        for (int j=0;j<=1;j++) {
            int l=i;
            int h=i+j;
            while (l>=0&&h<n&&s[l]==s[h]) {
                int len=h-l+1;
                if(p.second<len) {
                    p.first=l;
                    p.second=len;
                }
                l--;h++;
            }
        }
    }
    return s.substr(p.first,p.second);
}

// Given an array arr[] of size n, the task is to divide it into two sets S1 and S2 such that the absolute difference between their sums is minimum. 
// If there is a set S with n elements, then if we assume Subset1 has m elements, Subset2 must have n-m elements and the value of 
// abs(sum(Subset1) - sum(Subset2)) should be minimum.
auto partition_set(const vector<int>& arr) -> int {
    // time O(n*S)
    // space (S)
    int S=accumulate(arr.begin(),arr.end(),0);
    vector<bool> dp(S+1);
    dp[0]=true;
    for (int x : arr) {
        for (int sum=S;sum>=x;sum--) {
            dp[sum]=dp[sum]||dp[sum-x];
        }
    }
    int mn=S;
    for (int sum=0;sum<=S/2;sum++) {
        if (dp[sum]) {
            mn=min(mn,abs(S-sum)-sum);
        }
    }
    return mn;
}

auto job_scheduling_v1(vector<int>& startTime, vector<int>& endTime, vector<int>& profit) -> int {
    // Time O(n log n)
    int n = profit.size();
    vector<tuple<int,int,int>> jobs(n);
    for (int i = 0; i < n; i++) {
        jobs[i] = {startTime[i], endTime[i], profit[i]};
    }
    sort(jobs.begin(), jobs.end());
    vector<int> f(n);
    function<int(int)> dfs = [&](int i) -> int {
        if (i >= n) return 0;
        if (f[i]) return f[i];
        auto& [_, e, p] = jobs[i];
        tuple<int,int,int> t{e, 0, 0};
        int j = lower_bound(
            jobs.begin() + i + 1, jobs.end(), t, 
            [&](auto& l, auto& r) { return get<0>(l) < get<0>(r);}) - jobs.begin();
        int ans = max(dfs(i+1), p + dfs(j));
        f[i] =ans;
        return ans;
    };
    return dfs(0);
}

auto job_scheduling_v2(vector<int>& startTime, vector<int>& endTime, vector<int>& profit) -> int {
    // Time O(n log n)
    // Space O(n)
    int n = profit.size();
    vector<tuple<int,int,int>> jobs(n);
    for (int i = 0; i < n; i++) {
        jobs[i] = {endTime[i], startTime[i], profit[i]};
    }
    sort(jobs.begin(), jobs.end());
    vector<int> dp(n+1);
    for (int i = 0; i < n; i++) {
        auto& [_, s, p] = jobs[i];
        int j = upper_bound(jobs.begin(), jobs.begin() + i, s, [&](int x, auto& p) { return x < get<0>(p); }) - jobs.begin();
        dp[i+1] = max(dp[i], p + dp[j]);
    }
    return dp[n];
}

int main() {

    DP_PROBELMS problem = DP_PROBELMS::PARTITION_SET;

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
        case KNAPSACK01: {
            vector<int> val = {1, 2, 3};
            vector<int> wt = {4, 5, 1};
            int W = 4;
            cout<<knapsack_opt(W, val, wt)<<endl;
            break;
        }
        case NUMBER_BLAST: {
            vector<int64_t> A = {4, 2, 3, 1, 5, 9, 3, 7, 3, 4, 8, 2};
            cout<<number_blast_suffix(A)<< "\n";
            break;
        }
        case PERFECR_SQAURE: {
            int n=6;
            cout<<min_square_bfs(n)<<'\n';
            break;
        }
        case LONGEST_PALINDROMIC_SUBSTRING: {
            string s = "HYTBCABADEFGHABCDEDCBAGHTFYW1234567887654321ZWETYGDE";
            cout<<longest_pal_subs_center(s)<<'\n';
            break;
        }
        case PARTITION_SET: {
            vector<int> arr = {1, 6, 11, 5};
            cout<<partition_set(arr)<<'\n';
            break;
        }
        default:
            break;

    }
    return 0;
}