#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <functional>
using namespace std;

enum ARRAY_PROBLEMS {
    // EASY
    // MEDIUM
    THREE_SUM=0,
    ARRAY_CHANGE,
    KTH_LARGEST,
    // HARD
    COUNT_SMALLER_NUM,
    INSERT_INTERVAL,
    TRAPPING_RAIN_WATER_II,
};

auto threesum(vector<int>& arr) -> vector<vector<int>> {
    // time O(n^2)
    // space O(logn)
    int n=arr.size();
    vector<vector<int>> ans;
    sort(arr.begin(),arr.end());
    for (int i=0;i<n-2&&arr[i]<=0;i++) {
        if (i&&arr[i]==arr[i-1]) continue;
        int j=i+1;
        int k=n-1;
        while (j<k) {
            int x=arr[i]+arr[j]+arr[k];
            if (x<0) {
                j++; 
            } else if (x>0) {
                k--;
            } else {
                ans.push_back({arr[i],arr[j++],arr[k--]});
                while (j<k&&arr[j]==arr[j-1])j++;
                while (j<k&&arr[k]==arr[k+1])k--;
            }
        }
    }
    return ans;
}

auto trheesum_target(vector<int>& a, int target) -> bool {
    // time O(n^2)
    // space O(logn)
    int n=a.size();
    sort(a.begin(),a.end());
    for (int i=0;i<n-2;i++) {
        int x=target-a[i];
        int j=i+1;
        int k=n-1;
        while (j<k) {
            int y=a[j]+a[k];
            if (y<x) {
                j++;
            } else if (y>x) {
                k--;
            } else {
                return true;
            }
        }
    }
    return false;
}

auto array_change(vector<int> arr, vector<vector<int>>& operations) -> vector<int> {
    // time O(n+m)
    // space O(n)
    int n=arr.size();
    unordered_map<int,int> mp;
    for (int i=0;i<n;i++) {
        mp[arr[i]]=i;
    }
    for (auto& op : operations) {
        int x=op[0], y=op[1];
        arr[mp[x]] = y;
        mp[y]=mp[x];
    }
    return arr;
 }

 auto count_smaller(vector<int>& a) -> vector<int> {     
    struct bit {
        int n; vector<int> data;
        void init(int _n) { n = _n; data.resize(_n+1); }
        void update(int k) {
            while (k <= n) {
                data[k]++;
                k += (k & -k);
            }
        }
        int query(int k) {
            int sum = 0;
            while (k > 0) {
                sum += data[k];
                k -= (k & -k);
            }
            return sum;
        }
    };

    unordered_set<int> tt(a.begin(), a.end());
    vector<int> all(tt.begin(), tt.end());
    sort(all.begin(), all.end());
    int n = all.size();
    bit b;
    b.init(n);
    unordered_map<int,int> m;
    for (int i = 0; i < n; i++) m[all[i]]=i+1;
    vector<int> ans(a.size());
    for (int i = (int)a.size()-1; i >= 0; i--) {
        int x = m[a[i]];
        b.update(x);
        ans[i] = b.query(x-1);
    }
    return ans;
 }

 auto insert_interval(vector<vector<int>>& ii, vector<int>& ni) -> vector<vector<int>> { 
    auto run_loop = [&]() -> vector<vector<int>> {
        int n = ii.size();
        vector<vector<int>> ans;
        int i = 0;
        while (i < n && ii[i][1] < ni[0]) {
            ans.push_back(ii[i]);
            i++;
        }
        while (i < n && ni[1] >= ii[i][0]) {
            ni[0] = min(ni[0], ii[i][0]);
            ni[1] = max(ni[1], ii[i][1]);
        }
        ans.push_back(ni);
        while (i < n) {
            ans.push_back(ii[i++]);
        }
        return ans;
    };

    auto insert_loop = [&]() -> vector<vector<int>> {
        ii.push_back(ni);
        int n = ii.size();
        sort(ii.begin(), ii.end());
        vector<vector<int>> ans{ii[0]};
        for (int i = 1; i < n; i++) {
            if (ans.back()[1] < ii[i][0]) {
                ans.push_back(ii[i]);
            } else {
                ans.back()[1] = max(ans.back()[1], ii[i][1]);
            }
        }
        return ans;
    };

    return insert_loop();
 }

 auto trap_rain_water(vector<vector<int>>& H) {
    // Time O(m x n x log(mxn))
    // Space O(m x n )
    using tii = tuple<int,int,int>;
    priority_queue<tii, vector<tii>, greater<tii>> pq;
    int m = H.size(), n = H[0].size();
    bool vis[m][n];
    memset(vis, 0, sizeof(vis));
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            if (i == 0 || i == m-1 || j == 0 || j == n-1) {
                pq.push({H[i][j], i, j});
                vis[i][j] = true;
            }
        }
    }
    int dir[5]{-1,0,1,0,-1};
    int ans = 0;
    while (!pq.empty()) {
        auto [h, i, j] = pq.top();
        pq.pop();
        for (int k = 0; k < 4; k++) {
            int x = i + dir[k], y = j + dir[k+1];
            if (x >= 0 && x < m && y >= 0 && y < n && !vis[x][y]) {
                ans += max(0, h - H[x][y]);
                pq.push({max(h, H[x][y]), x, y});
                vis[x][y] = true;
            }
        }
    }
    return ans;
 }
int find_kth_largest(vector<int>& A, int k) {
    int n = A.size();
    k = n-k;
    function<int(int,int)> quickselect = [&](int l, int r) -> int {
        if (l == r) {
            return A[l];
        }
        int x = (l + r) >> 1;
        x = A[x];
        int i = l-1, j = r +1;
        while (i < j) {
            while (A[++i] < x){
            }
            while (A[--j] > x) {
            }
            if (i < j)
                std::swap(A[i], A[j]);
        }
        if (j < k) {
                return quickselect(j+1, r);
            }
        return quickselect(l, j);
    };
    return quickselect(0, n-1);
}


int main() {

    ARRAY_PROBLEMS problem = ARRAY_PROBLEMS::ARRAY_CHANGE;

    switch (problem) {
        case THREE_SUM: {
            vector<int> a{-1,0,1,2,-1,-4};
            auto aa=threesum(a);
            for (auto b : aa) {
                for (int x : b) {
                    cout<<x<<' ';
                }
                cout<<'\n';
            }
            vector<int> arr = { 1, 4, 45, 6, 10, 8 }; 
            int target = 13; 
            if(trheesum_target(arr, target))
                cout << "true\n";
            else
                cout << "false\n";
            break;
        }
        case ARRAY_CHANGE: {
            vector<int>nums{1,2,4,6};
            vector<vector<int>>operations{{1,3},{4,7},{6,1}};
            auto ans=array_change(nums,operations);
            for (auto x : ans) {
                cout<<x<<" ";
            }
            cout<<'\n';
            break;
        }
        default:
            break;
    }

    return 0;
}