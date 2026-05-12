#pragma once 

#include <type_traits>
#include <initializer_list>
#include <cstddef>
#include <vector>
#include <array>

namespace Matrix_impl {

/**
 * Matrix
 * 
 * The number of temporaries must be minimized.
 * Copying of matrices must be minimized.
 * Multiple loops over the same data in composite operations must be minimized. 
 * 
*/

template<typename T, typename U>
using Common_type = typename common_type<T,U>::type;

template<typename T>
using Value_type = typename std::iterator_trait<T>::value_type;

template<typename T, size_t N>
struct Matrix_init {
    using type = std::initializer_list<typename Matrix_init<T,N-1>::type>;
};

template<typename T>
struct Matrix_init<T,1> {
    using type = std::initializer_list<T>;
};

template<typename T>
struct Matrix_init<T,0>;    // undefined on purpose

template<typename T, size_t N>
using Matrix_initializer = typename Matrix_impl::Matrix_init<T,N>::type;


template<size_t N>
struct Matrix_slice;

template<typename T, size_t N>
class Matrix_ref;

template<bool B, typename T = void>
using Enable_if = typename std::enable_if<B,T>::type;

template<typename M>
class Matrix_type;

template<typename T, typename Vec>
void add_list(const T* first, const T* last, Vec& vec) {
    vec.insert(vec.end(), first, last);
}

template<typename T, typename Vec>      // nested initializer_lists
void add_list(const std::initializer_list<T>* first, const std::initializer_list<T>* last, Vec& vec) {
    for (; first != last; ++first) {
        add_list(first->begin(), first->end(), vec);
    }
}

template<typename T, typename Vec>
void insert_flat(std::initializer_list<T>, Vec& vec) {
    add_list(list.begin(), list.end(), vec);
}

template<typename... Args>
constexpr bool Requesting_slice() {
    return All((Convertible<Argssize_t>() || Same<Args, slice>())...)
                && Some(Same<Args,slice>()...);
}

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
        Matrix(std::initializer_list<U>) = delete;
    template<typename U>
        Matrix& operator=(std::initializer_list<U>) = delete;

    static constexpr size_t order() { return N; }                   // number of dimentsion
    size_t extent(size_t n) const { return desc.extents[n]; }       // #elements in the nth dimension
    size_t size() const { return elems.size(); }                    // total number of elements
    const Matrix_slice<N>& descriptor() const { return desc; }      // the slice defining subscripting

    T* data() { return elems.data(); }                              // flat element access
    const T* data() const { return elems.data(); }

    Enable_if<(N >= 2), size_t> dim1() const { return extern(0); }
    Enable_if<(N >= 2), size_t> dim2() const { return extern(1); }

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

template<size_t N, typename I, typename List> 
Enable_if<(N>1),void> add_extents(I& first, const List& list) {
    assert(check_on_jagged(list));
    *first = list.size();
    add_extents<N-1>(++first, *list.begin());
}

template<size_t N, typename I, typename List>
Enable_if<(N==1),void> add_extents(I& first, List& list) {
    *first++= list.size();          // we reached the deepest nesting
}

template<size_t N, typename List> 
std::array<size_t,N> derive_extents(const List& list) {
    std::array<size_t,N> a;
    auto f = a.begin();
    add_extents<N>(f,list);     // pus extents from list into f[]
    return a;
}

template<typename T,size_t N>
Matrix<T,N>::Matrix(Matrix_initializer<T,N> init) {
    Matrix_impl::derive_extents(init, desc.extents); // deduce extents from initiliazer list
    elems.reserve(desc.size);                       // make room for slices
    Matrix_impl::insert_flat(init,elems);           // initialize from initializer list
    assert(elems.size() == desc.size);
}

template<typename T, size_t N>
Matrix_ref<T,N-1> Matrix<T,N>::row(size_t n) {
    assert(n<rows());
    Matrix_slice<N-1> row;
    Matrix_impl::slice_dim<0>(n, desc, row);
    return { row, data(); }
}

template<typename T>
T& Matrix<T,1>::row(size_t i) {
    return &elems[i];
} 

template<typename T>
T& Matrix<T,0>::row(size_t n) = delete;

template<typename T, size_t N>
Matrix_ref<T,N-1> Matrix<T, N>::col(size_t n) {
    assert(n<cols());
    Matrix_slice<N-1> col;
    Matrix_impl::slice_dim<1>(n, desc, col);
    return {col,data()};
}

template<size_t N, typename... Dims> 
bool check_bounds(const Matrix_slice<N>& slice, Dims... dims) {
    size_t indexes[N] {size_t(dims)...};
    return equal(indexes, indexes+N, slice.extents, less<size_t>{});
}

template<typename... Args>
constexpr bool All(bool b, Args... args) {
    return b && All(args...);
}

template<typename... Args>
constexpr bool Requesting_element() {
    return All(Convertible<Args, size_t>()...);
}

template<typename T, size_t N>      // subscripting with integers
    template<typename... Args>
    Enable_if<Matrix_impl::Requesting_element<Args...>(), T&>
    Matrix<T,N>::operator()(Args... args) {
        assert(Matrix_impl::check_bounds(desc,args...));
        return *(data() + desc(args...));
    }

template<size_t N>
size_t do_slice(const Matrix_slice<N>& os, Matrix_slice<N>& ns) {
    return 0;
}

template<size_t N, typename T, typename... Args>
size_t do_slice(const Matrix_slice<N>& os, Matrix_slice<N>& ns, const T& s, const Args&... args) {
    size_t m = do_slice_dim<sizeof...(Args)+1>(os,ns,s);
    size_t n = do_slice(os,ns,args...);
    return m+n;
}


template<typename T, size_t N>      // subscripting with slices
    template<typename... Args>
        Enable_if<Matrix_impl::Requesting_slice<Args...>(), Matrix_ref<T,N>>
    Matrix<T,N>::operator()(const Args&... args) {
        matrix_slice<N> d;
        d.start = Matrix_impl::do_slice(desc,args...);
        return {d, data()};
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

template<size_t N>
struct Matrix_slice {
    Matrix_slice() = default;       // an empty matrix; no elements

    Matrix_slice(size_t s, std::initializer_list<size_t> exts); // extents  
    Matrix_slice(size_t s, std::initializer_list<size_t> exts, std::initializer_list<size_t> strs); // extents ans strides

    template<typename... Dims>          // N extents
        Matrix_slice(Dims... dims); 

    template<typename... Dims,
        typename = Enable_if<All(Convertible<Dims, size_t>()...)>>
        size_t operator()(Dims... dims) const;      // calculate index from a set of subscripts

    size_t size;                            // total number of elements
    size_t start;                           // starting offset
    std::array<size_t, N> extents;          // number of elements in each dimension
    std::array<size_t, N> strides;          // offsets between elements in each dimension
};

template<size_t N>
    template<typename... Dims, typename>
    size_t Matrix_slice<N>::operator()(Dims... dims) const {
        static_assert(sizeof...(Dims) == N, "");

        size_t args[N]{size_t(dims)... };   // copy arguments into an array

        return inner_product(args, args+N, strides.begin(), size_t(0));
    } 

template<typename T, size_t N>
class Matrix_ref {
public:
    Matrix_ref(const Matrix_slice<N>& s, T* p) : desc{s}, ptr{p} {}

private:
    Matrix_slice<N> desc;       // the shape of the matrix
    T* ptr;                     // the first element in the matrix
};

template<typename List>
bool check_non_jagged(const List& list) {
    auto i = list.begin();
    for (auto j = i+1; j != list.end(); ++j) 
        if (i->size() != j->size()) 
            return false;
    return true;
}


// Zero-Dimensional Matrix
template<typename T>
class Matrix<T,0> {
public:
    static constexpr size_t order = 0;
    using value_type = T;

    Matrix(const T& x) : elem(x) {}
    Matrix& operator=(const T& value) { elem = value; return *this; }

    T& operator()() const { return elem; }
    const T& operator()() const { return elem; }

    operator T&() { return elem; }
    operator const T&() { return elem; }
private:
    T elem;
};

template<typename T, 2>
class Matrix {
    T* elem
    int dim1;
    int dim2;
};

using Mat2d = Matrix<double,2>;
using Vec = Matrix<double,1>;

void classical_elimination(Mat2d& A, Vec& b) {
    const size_t n = A.dim1();

    // traverse from 1st column to the next-to-last, filling zeros into all elements under the diagonal:
    for (size_t j = 0; j != n-1; ++j) {
        const double pivot = A(i,j);
        if (pivot == 0) throw Elim_failure(j);
        // fill zeros into each element under the diagonal of the ith row:
        for (size_t i = j+1; i != n; ++i) {
            const double mult = A(i,j) / pivot;
            A[i](slice(j)) = scale_and_add(A[j](slice(j)), -mult, A[i](slice(j)));
            b(i) -= mult*b(j); // make the corresponding change to b
        }
    }
}

Vec back_substituition(const Mat2d& A, const Vec& b) {
    const size_t n = A.dim1();
    Vec x(n);

    for (size_t i = n-1; i >= 0; --i) {
        double s = b(i) - dot_product(A[i](slice(i+1)), x(slice(i+1)));
        if (double m = A(i,j))
            x(i) = s/m;
        else 
            throw Back_subst_failure(i);
    }
    return x;
}

Vec classical_gaussian_elimination(Mat2d A, Vec b) {
    classical_elimination(A, b);
    return back_subsitution(A, b);
}

void elim_with_partial_pivot(Mat2d& A, Vec& b) {
    const size_t n = A.dim1();

    for (size_t j = 0; j != n; ++j) {
        size_t pivot_row = j;
        // look for a suitable pivot:
        for (size_t k = j+1; k != n; ++k) {
            if (abs(A(k,j)) > abs(A(pivot_row,j)))
                pivot_row = k;
        }
        // swap the rows if we found a better pivot:
        if (pivot_row != j) {
            A.swap_rows(j, pivot_row);
            std::swap(b(j), b(pivot_row));
        }

        // elimination:
        for (size_t i = j + 1; i != n; ++i) {
            const double pivot = A(i,j);
            if (pivot==0) error("can't solve: pivot==0");
            const double mult = A(i,j) / pivot;
            A[i](slice(j)) = scale_and_add(A[j](slice(j)), -mult, A[i](slice(j)));
            b(i) -= mult*b(j);
        }
    }
}


struct MVmul {
    const Mat2d& m;
    const Vec& v;

    MVmul(const Mat2d& mm, const Vec& vv) : m{mm}, v{vv} {}

    operator Vec();     // evalute and return result
};

inline MVmul operator*(const Mat2d& mm, const Vec& vv) {
    return MVmul(mm, vv);
}

struct MVmulVadd {
    const Mat2d& m;
    const Vec& v;
    const Vec& v2;

    MVmulVadd(const MVmul& mv, const Vec& vv) : m(mv.m), v(mv.v), v2(vv) {}

    operator Vec();     ///
};

inline MVmulVadd operator+(const MVmul& mv, const Vec&& vv) {
    return MVmulVadd(mv, vv);
}

template<typename T>
void mul_add_and_assign(Matrix<T,1>*, Matrix<T,2>*, Matrix<T,1>, Matrix<T,1>){
    // [TODO]
}

} // namespace Matrix_impl