#ifndef SIKRADIO_FIXED_SIZE_QUEUE_H
#define SIKRADIO_FIXED_SIZE_QUEUE_H

#include <deque>

template <typename T>
class FixedSizeFIFODeque {
    std::deque<T> _fifo_deque;
    size_t _max_deque_size;
public:
    explicit FixedSizeFIFODeque(size_t max_queue_size) : _max_deque_size(max_queue_size) {}

    void push(const T& t) {
        if(_fifo_deque.size() == _max_deque_size) {
            _fifo_deque.pop_front();
        }
        _fifo_deque.push_back(t);
    }
    const T& at(size_t index) const {
        return _fifo_deque.at(index);
    }
    size_t size() const {
        return _fifo_deque.size();
    }

};

#endif //SIKRADIO_FIXED_SIZE_QUEUE_H
