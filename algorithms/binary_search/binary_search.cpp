#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

enum class Problem {
    // Hard
    K_SMALLEST_PAIR=0,
    MAKE_ARRAY_STRICKLY_INCREASING,
};

auto smallest_dist_pair(vector<int>& arr, int k) -> int {
    int n = arr.size();
    sort(arr.begin(), arr.end());
    int l = 0, r = arr.back() - arr.front();

    auto count = [&](int dist) -> int {
        int cnt = 0;
        for (int i =0 ; i < n; i++) {
            int t = arr[i] - dist;
            int j = lower_bound(arr.begin(), arr.end(), t) - arr.begin(); 
            cnt += i - j;
        }
        return cnt;
    };

    while (l < r) {
        int mid = (l + r) >> 1;
        if (count(mid) >= k) {
            r = mid; 
        } else {
            l = mid + 1;
        }
    }
    return l;
}

auto make_array_increasing(vector<int>& arr1, vector<int>& arr2) -> int {
    sort(arr2.begin(), arr2.end());
    arr2.erase(unique(arr2.begin(), arr2.end()), arr2.end());
    int n = arr1.size();
    const int inf = 1<<30;
    arr1.insert(arr1.begin(), -inf);
    arr1.push_back(inf);
    vector<int> f(n,inf);
    f[0] = 0;
    for (int i = 1; i < n; i++) {
        if (arr1[i-1] < arr1[i]) {
            f[i] = f[i-1];
        }
        int j = lower_bound(arr2.begin(), arr2.end(), arr1[i]) - arr2.begin();
        for (int k = 1; k <= min(i-1, j); k++) {
            if (arr1[i-k-1] < arr2[j-k]) {
                f[i] = min(f[i], f[i-k-1] + k);
            }
        }
    }
    return f[n-1] >= inf ? -1 : f[n-1];
}

int main() {
    Problem problem = Problem::K_SMALLEST_PAIR;

    switch (problem) {
        case Problem::K_SMALLEST_PAIR: {

        }
        default:
            break;
    }

    return 0;
}