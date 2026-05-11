#pragma once 

#include <type_traits>
#include <initializer_list>
#include <cstddef>
#include <vector>

// Matrix<T, N> 

namespace Matrix_impl {
    

template<typename T, typename U>
using Common_type = typename common_type<T,U>::type;

template<typename T>
using Value_type = typename std::iterator_trait<T>::value_type;

template<typename T, size_t N>
class Matrix_initializer;

class Matrix_slice;

template<typename T, size_t N>
class Matrix_ref;

template<typename T, typename M>
class Enable_if;

template<typename M>
class Matrix_type;

template<typename T, size_t N> 
class Matrix {
public:
    static constexpr size_t order = N;
    using value_type = T;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    Matrix() = default;
    Matrix(Matrix&&) = default;                      // move
    Matrix& operator=(Matrix&&) = default;
    Matrix(Matrix const&) = default;                // copy
    Matrix& operator=(Matrix const&) = default;
    ~Matrix() = default;


    template<typename U>
        Matrix(const Matrix_ref<U, N>&);
    template<typename U>
        Matrix& operator=(const Matrix_ref<U,N>&);

    template<typename... Exts>
        explicit Matrix(Exts... exts);
    
    Matrix(Matrix_initializer<T, N>);
    Matrix& operator=(Matrix_initializer<T,N>);

    template<typename U>
        Matrix(initializer_list<U>) = delete;
    template<typename U>
        Matrix& operator=(initializer_list<U>) = delete;

    static constexpr size_t order() { return N; }                   // number of dimentsion
    size_t extent(size_t n) const { return desc.extents[n]; }       // #elements in the nth dimension
    size_t size() const { return elems.size(); }                    // total number of elements
    const Matrix_slice<N>& descriptor() const { return desc; }      // the slice defining subscripting

    T* data() { return elems.data(); }                              // flat element access
    const T* data() const { return elems.data(); }

    template<typename... Args>      // m(i,j,k) subscripting with integers
        std::enable_if<Matrix_impl::Requesting_element<Args...>(), T&>
        operator()(Args... args);
    template<typename... Args>
        Enable_if<Matrix_impl::Requesting_element<Args...>(), const T&>
            operator()(Args... args) const;
    template<typename... Args>      // m(s1,s2,s3) subscripting with slices
        Enable_if<Matrix_impl::Requesting_slice<Args...>(), Matrix_ref<T,N>>
        operator()(const Args&... args);
    template<typename... Args>
        Enable_if<Matrix_impl::Requesting_slice<Args...>(), Matrix_ref<const T,N>>
        operator()(const Args&... args) const;

    Matrix_ref<T,N-1> operator[](size_t i) { return row(i); }  // m[i] row access
    Matrix_ref<const T,N-1> operator[](size_t i) const { return row(i); }

    Matrix_ref<T,N-1> row(size_t n);                // row access
    Matrix_ref<const T,N-1> row(size_t n) const;

    Matrix_ref<T,N-1> col(size_t n);                // col access
    Matrix_ref<const T,N-1> col(size_t n) const;

    template<typename F>
        Matrix& apply(F f);     // f(x) for every element x

    template<typename M, typename F>
        Matrix& apply(const M& m, F f);       // f(x, mx) for corresponding elements

    Matrix& operator=(const T& value);        // assigment with scalar

    Matrix& operator+=(const T& value);         // scalar addition
    Matrix& operator-=(const T& value);         // scalar subtraction
    Matrix& operator*=(const T& value);         // scalar multiplication
    Matrix& operator/=(const T& value);         // scalar division
    Matrix& operator%=(const T& value);         // scalar modulo


    template<typename M>                        // matrix addition
        Matrix& operator+=(const M& x);
    template<typename M>                        // matrix subtraction
        Matrix& operator-=(const M& x);


private:
    Matrix_slice<N> desc;
    std::vector<T> elems;
};

template<typename T, size_t N>
    template<typename... Exts>
    Matrix<T, N>::Matrix(Exts... exts)
        : desc{exts...}, 
          elems(desc.size) 
    {}

template<typename T, size_t N>
Matrix<T, N>::Matrix(Matrix_initializer<T, N> init) {
    Matrix_impl::derive_extents(init, desc.extents);
    elems.reserve(desc.size);
    Matrix_impl::insert_flat(init, elems);
    assert(elems.size() == desc.size)
}

template<typename T, size_t N>
    template<typename U>
    Matrix<T,N>::Matrix(const Matrix_ref<U,N>& x) 
        : desc{x.desc}, elems{x.begin(), x.end()} {

        static_assert(Convertible<U,T>(), ,"Matrix constructor: incompatible element types");
    }

template<typename T, size_t N>
    template<typename U>
    Matrix<T,N>& Matrix<T,N>::operator=(const Matrix_ref<U,N>& x) {
        static_assert(Convertible<U,T>(), ,"Matrix =: incompatible element types");

        desc = x.desc;
        elems.assign(x.begin(), e.end());
        return *this;
    }

template<typename T, size_t N>
Matrix_ref<T,N-1> Matrix<T,N>::operator[](size_t n) {
    return row(n);
}

template<typename T,size_t N>
Matrix<T,N>& Matrix<T,N>::operator+=(const T& val) {
    return apply([&](T& a) { a += val; });
}

template<typename T, size_t N>
    template<typename F>
    Matrix<T,N>& Matrix<T,N>::apply(F f) {
        for (auto& x : elems) f(x);
        return *this;
    }

template<typename T, size_t N>
Matrix<T,N> operator+(const Matrix<T,N>& m, const T& val) {
    Matrix<T,N> res = m;
    res += val;
    return res;
} 

template<typename T, size_t N>
    template<typename M>
    Enable_if<Matrix_type<M>(), Matrix<T,N>&> Matrix<T,N>::operator+=(const M& m) {
        static_assert(m.order() == N, ,"+=: mismatched Matrix dimensions");
        assert(same_extents(desc, m.descriptor())); // make sure sizes match
        return apply(m, [](T& a, Value_type<M>& b) { a+= b; });
    }

template<typename T, size_t N>
    template<typename M, typename F>
    Enable_if<Matrix_type<M>(), Matrix<T,N>&> Matrix<T,N>::apply(M& m, F f) {
        assert(same_extents(desc, m.descriptor()));
        for (auto i = begin(), j = m.begin(); i != end(); i++, j++) {
            f(*i, *j);
        }
        return *this;
    }

template<typename T, size_t N>
Matrix<T,N> operator+(const Matrix<T,N>& a, const Matrix<T,N>& b) {
    Matrix<T, N> res = a;
    res += b;
    return res;
}

template<typename T, typename T2, size_t N, 
    typename RT = Matrix<Common_type<Value_type<T>, Value_type<T2>>, N>>
    Matrix<RT,N> operator+(const Matrix<T,N>& a, const Matrix<T2,N>& b) {
        Matrix<RT,N> res = a;
        res += b;
        return res;
    }

template<typename T, size_t N>
Matrix<T,N> operator+(const Matrix_ref<T,N>& x, const T& n) {
    Matrix<T,N> res = x;
    res += n;
    return res;
}

template<typename T>
Matrix<T,2> operator*(const Matrix<T,1>& u, const Matrix<T,1>& v) {
    const size_t n = u.extent(0);
    const size_t m = v.extent(0);
    Matrix<T,2> res(n,m);               // an n-by-m matrix
    for (size_t i = 0; i != n; i++) {
        for (size_t j = 0; j != m; j++) {
            res(i,j) = u[i]*v[j];
        }
    }
    return res;
}

template<typename T>
Matrix<T,1> operator*(const Matrix<T,2>& m, const Matrix<T,1>& v) {

    assert(m.extent(1) == v.extent(0));

    const size_t n = m.extent(0);
    Matrix<T,1> res(n);
    for (size_t i = 0; i != n; i++) 
        for (size_t j = 0; j != n; j++) 
            res(i) += m(i,j) * v(j);
    return res;
}

template<typename T>
T dot_product(const Matrix_ref<T,1>& a, const Matrix<T,1>& b) {
    return inner_product(a.begin(), a.end(), b,begin(), 0.0);
}

template<typename T>
Matrix<T,2> operator*(const Matrix<T,2>& m1, const Matrix<T,2>& m2) {
    const size_t n = m1.extent(0);
    const size_t m = m1.extent(1);
    assert(m == m2.extent(0));          // columns must match rows

    const size_t p = m2.extent(1);
    Matrix<T,2> res(n,p);
    for (size_t i = 0; i != n; i++) 
        for (size_t j = 0; j != m; j++) 
            for (size_t k = 0; k != p; k++) 
                res(i,j) = m1(i,k) * m2(k,j);
                // or
                // res(i,j) = dot_product(m1[i], m2.col(j));
    return res;
}

struct slice {
    slice() : start(-1), length(-1), stride(1) {}
    explicit slice(size_t s) : start(s), length(-1), stride(1) {}
    slice(size_t s, size_t l, size_t n=1) : start(s), length(l), stride(n) {}

    size_t operator()(size_t i) const { return start+i*stride; }
    
    static slice all;

    size_t start;           // first index
    size_t length;          // numner of indices included (can be used for range checking)
    size_t stride;          // distance between elements in sequence 
};



} // namespace Matrix_impl