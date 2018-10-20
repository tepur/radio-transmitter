#ifndef SIKRADIO_MUTEX_WRAPPER_H
#define SIKRADIO_MUTEX_WRAPPER_H

#include <mutex>

template <typename T>
struct WithMutexWrapper {
    T val; 
    std::mutex mutex;
};

#endif //SIKRADIO_MUTEX_WRAPPER_H
