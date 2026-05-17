#pragma once 

#include <vector>
#include <complex>
#include <cmath>

namespace tt {


using cd = std::complex<double>;
const double PI = acos(-1);

/**
 * Cooley-Tukey FFT Divide and conquer
 * 
 * DFT evalutes polynomial at powers of an n-th root of unity.
 * 
 *              y_k = A(w_{n}^{k})
 * 
 * Split the polynomial into even and odd powers;
 *  
 *          A(x) = (a_0 + a_2 * x^2 + a_4 * x^ 4 + ...) + x(a_1 + a_3 * x^2 + a_5 * x^ 4)
 * 
 *          A_0(x) = a_0 + a_2 * x + a_4 * x^2 + ...
 *          A_1(x) = a_1 + a_3 * x + a_5 * x^2 + ...
 * 
 * Then:
 *         A(x) = A_0(x^2) + x * A_1(x^2)
 */

void fft(std::vector<cd>& a, bool invert) {
    int n = a.size();
    if (n == 1)
        return;
    
    std::vector<cd> a0(n/2), a1(n/2);
    for (int i = 0; 2*i < n; ++i) {
        a0[i] = a[2*i];
        a1[i] = a[2*i+1];
    }
    fft(a0, invert);
    fft(a1, invert);

    double ang = 2 * PI / n * (invert ? -1 : 1);
    cd w(1), wn(cos(ang), sin(ang));
    for (int i = 0; 2 * i < n; ++i) {
        a[i] = a0[i] + w * a1[i];
        a[i+n/2] = a0[i] - w * a1[i];
        if (invert) {
            a[i] /= 2;
            a[i+n/2] /= 2;
        }
        w *= wn;
    }
}

std::vector<int> multiply(std::vector<int>& a, std::vector<int>& b) {
    std::vector<cd> fa(a.begin(), a.end()), fb(b.begin(), b.end());
    int n = 1;
    while (n < a.size() + b.size()) 
        n <<= 1;
    fa.resize(n);
    fb.resize(n);

    fft(fa, false);
    fft(fb, false);
    for (int i = 0; i < n; ++i) 
        fa[i] *= fb[i];
    fft(fa, true);

    std::vector<int> result(n);
    for (int i = 0; i < n; ++i) 
        result[i] = std::round(fa[i].real());
    return result;
}

}