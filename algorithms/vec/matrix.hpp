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



