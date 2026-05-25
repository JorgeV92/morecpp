#pragma once 

/*
*
* String Hasing 
*
* For problems when we want to compare strings effieciently 
* 
* We need a hash funciton to convert a string into an integer identity. 
* Given this method for two strings the hash(s) == hash(t) if s == t.
*
*
* Calculate the hash of a string 
*
*     hash(s) = s[0] + s[1] * p + s[2] * p^2 + ... + s[n-1] * p^{n-1} mod m 
*      
*         = \sum_{i=0}^{n-1} s[i] * p[i] mod m 
*
* where p and m are some positive number. 
*
* For exmaple we can make p to be a prime number roughly equal to the number of characters
* in the input Alphabet. We set p = 31 for only lowercase English letters. 
*
* We also need to choice a large number for m to have the probability of collosion high ~ 1 / m 
*/


#include <cstdint>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_set>

int64_t compute_hash(const std::string& s) {
  const int p = 31;
  const int m  = 1e9 + 9;
  std::int64_t hash_value = 0;
  int64_t p_pow = 1;
  for (char c : s) {
    hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
    p_pow = (p_pow * p) % m;
  }
  return hash_value;
}

// Search for duplicate strings in an array of strings
// O (mn + nlogn)
std::vector<std::vector<int>> group_identical_strings(const std::vector<std::string>& s) {
  int n = s.size();
  std::vector<std::pair<int64_t, int>> hashes(n);
  for (int  i = 0; i < n; ++i) {
    hashes[i] = {compute_hash(s[i]), i};
  }

  std::sort(hashes.begin(), hashes.end());
  std::vector<std::vector<int>> groups;

  for (int i = 0; i < n; ++i) {
    if (i == 0 || hashes[i].first != hashes[i-1].first) 
        groups.emplace_back();
    groups.back().push_back(hashes[i].second);
  }
  return groups;
}

// Number of different substrings in a string 
int count_unique_substrings(const std::string& s) {
  int n = s.size();
  const int p = 31;
  const int m = 1e9+9;
  std::vector<int64_t> p_pow(n);
  p_pow[0] = 1;
  for (int i = 1; i < n; ++i) 
    p_pow [i] = (p_pow[i-1]*p) % m;
  
  std::vector<int64_t> h(n+1, 0);
  for (int i = 0; i < n; ++i) 
    h[i+1] = (h[i] + (s[i] - 'a' + 1) * p_pow[i]) % m;

  int cnt = 0;
  for (int l = 1; l <= n; ++l) {
    std::unordered_set<int64_t> hs;
    for (int i = 0; i <= n-l; ++i) {
      int64_t cur_h = (h[i+l] + m - h[i]) % m;
      cur_h = (cur_h * p_pow[n-i-1]) % m;
      hs.insert(cur_h);
    }
    cnt += hs.size();
  }
  return cnt;
}


