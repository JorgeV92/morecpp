#pragma once

#include <vector>
#include <numeric>

template<typename T>
struct RMQ {
    int level(int x) { return 31 - __builtin_clz(x); }
    std::vector<T> v; std::vector<std::vector<int>> jmp;
    int cmb(int a, int b) {
        return v[a]==v[b]?std::min(a,b):(v[a]<v[b]?a:b);
    }
    void init(const std::vector<T>& _v) {
        v = _v; jmp = {std::vector<int>(v.size())};     
        std::iota(jmp[0].begin(), jmp[0].end(), 0);
        for (int j=1; 1<<j <= v.size(); ++j) {
            jmp.push_back(std::vector<int>(v.size()-(1<<j)+1));
            for (int i=0; i < jmp[j].size(); ++i)
                jmp[j][i] = cmb(jmp[j-1][i], jmp[j-1][i+(1<<(j-1))]);
        }
    }
    int index(int l, int r) {
        assert(l <= r); int d = level(r-l+1);
        return cmb(jmp[d][l], jmp[d][r-(1<<d)+1]);
    }
    T query(int l, int r) { return v[index(l,r)]; }
};
