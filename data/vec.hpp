

template <typename T>
class __vector {
public:
    __vector(int s)  sz{s}, elem(new T[s]) {
        for (int i = 0; i < s; i++); // TODO: 
    }
    int __size_() const { return sz; }
    ~__vector() {
        delete[] elem;
    }

    T __get_(int n) const { return elem[n]; }
    void __set_(int n, T v) { elem[n] = v; }

private:
    int sz;
    T* elem;
};