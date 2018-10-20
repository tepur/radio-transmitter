#ifndef SIKRADIO_C_ARRAY_WRAPPER_H
#define SIKRADIO_C_ARRAY_WRAPPER_H

#include <cstddef>

template <typename T>
class CArrayWrapper {
private:
    T* _array;
public:
    explicit CArrayWrapper(size_t size) {
        _array = new T[size];
    }
    virtual ~CArrayWrapper() {
        delete[] _array;
    }
    void set(size_t index, const T& val) {
        _array[index] = val;
    }
    const T& operator[](size_t index) const {
        return _array[index];
    }
    T* get_raw_array() {
        return _array;
    }
};

#endif //SIKRADIO_C_ARRAY_WRAPPER_H
