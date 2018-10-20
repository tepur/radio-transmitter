#ifndef SIKRADIO_TRANSMITTER_H
#define SIKRADIO_TRANSMITTER_H

#include <string>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <regex>
#include <mutex>
#include <future>
#include <set>
#include "audio.h"
#include "fixed_size_fifo_deque.h"
#include "mutex_wrapper.h"
#include "socket.h"

class Transmitter {
private:
    using rtime_type = std::chrono::duration<long, std::milli>;

    static constexpr const int ADDITIONAL_PACKET_INFO_SIZE = 50, ALBUM_NUMBER = 386430;

    std::string _station_name = "Nienazwany Nadajnik";
    std::string _mcast_addr;
    unsigned short _data_port = 20000 + (ALBUM_NUMBER % 10000);
    unsigned short _ctrl_port = 30000 + (ALBUM_NUMBER % 10000);
    rtime_type _rtime;
    size_t _fsize = 128000;
    size_t _psize = 512;

    WithMutexWrapper<std::byte*> _send_buffer;

    UDPSocket _send_sock;

    std::atomic<bool> _transmitter_is_on = false;
    std::atomic<uint64_t> _n_of_bytes_read = 0;

    WithMutexWrapper<FixedSizeFIFODeque<std::byte>*> _last_bytes_queue;

    std::future<void> _rexmit_task;

    const uint64_t _default_session_id = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

    void send_packet_to_mcast(const AudioPacket &packet);
    void set_listener_socket(UDPSocket &sock);
    void listen_to_receivers();
    void init_server();
    void set_socket_timeout(UDPSocket& sock, const std::chrono::system_clock::time_point& start_time,
                            const rtime_type& time_for_work);

    inline static bool any_time_left(const std::chrono::system_clock::time_point& start_time,
                                     const rtime_type& time_for_work) {
        return (std::chrono::system_clock::now() - start_time) < time_for_work;
    }

    inline size_t whole_packet_size() {
        return _psize + ADDITIONAL_PACKET_INFO_SIZE;
    }

public:
    Transmitter() : _rtime(rtime_type(250)) {
        _send_buffer.val = nullptr;
        _last_bytes_queue.val = nullptr;
    }

    virtual ~Transmitter() {
        delete[] _send_buffer.val;  //sprawdzać, czy nie null?
        delete _last_bytes_queue.val;
    }

    inline void set_mcast_addr(const char* str) {
        if(!_transmitter_is_on)
            _mcast_addr = str;
    }
    inline void set_data_port(const char* str) {
        if(!_transmitter_is_on)
            _data_port = static_cast<unsigned short>(std::stoi(str));
    }
    inline void set_ctrl_port(const char* str) {
        if(!_transmitter_is_on)
            _ctrl_port = static_cast<unsigned short>(std::stoi(str));
    }
    inline void set_psize(const char* str) {
        if(!_transmitter_is_on)
            _psize = std::stoul(str);
    }
    inline void set_fsize(const char* str) {
        if(!_transmitter_is_on)
            _fsize = std::stoul(str);
    }
    inline void set_rtime(const char* str) {
        if(!_transmitter_is_on)
            _rtime = rtime_type(std::stol(str));
    }
    inline void set_name(const char* str) {
        if(!_transmitter_is_on)
            _station_name = str;
    }
    inline bool mcast_addr_not_set() {
        return _mcast_addr.empty();
    }

    void run();
    void init();
    void rexmit_packets(std::set<uint64_t> packet_first_byte_nums);
};


#endif //SIKRADIO_TRANSMITTER_H
