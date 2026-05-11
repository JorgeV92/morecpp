#pragma once 

#include <type_traits>
#include <initializer_list>
#include <cstddef>
#include <vector>

// Matrix<T, N> 

template<typename T, size_t N>
class Matrix_initializer;

class Matrix_slice;

template<typename T, size_t N>
class Matrix_ref;

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
