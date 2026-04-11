#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
using namespace std;

enum ARRAY_PROBLEMS {
    // EASY
    // MEDIUM
    THREE_SUM=0,
    ARRAY_CHANGE,
    // HARD
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