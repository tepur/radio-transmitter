#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

#include "socket.h"

UDPSocket::~UDPSocket() {
    if(_sock != (-1)) {
        close(_sock);
    }
}

void UDPSocket::socket_wrapper(int domain, int type, int protocol) {
    _sock = socket(domain, type, protocol);
    if(_sock < 0) {
        std::cerr << "Error in socket: " << strerror(errno) << std::endl;
        throw SocketException();
    }
}

void UDPSocket::setsockopt_wrapper(int level, int option_name, const void *option_value, socklen_t option_len) {
    if(setsockopt(_sock, level, option_name, option_value, option_len) < 0) {
        std::cerr << "Error in setsockopt: " << strerror(errno) << std::endl;
        throw SocketException();
    }
}

void UDPSocket::inet_aton_wrapper(const char *cp, struct in_addr *inp) {
    if (inet_aton(cp, inp) == 0) {
        std::cerr << "Error in inet_aton: " << strerror(errno) << std::endl;
        throw SocketException();
    }
}

void UDPSocket::connect_wrapper(const struct sockaddr *address, socklen_t address_len) {
    if (connect(_sock, address, address_len) < 0) {
        std::cerr << "Error in connect: " << strerror(errno) << std::endl;
        throw SocketException();
    }
}

void UDPSocket::bind_wrapper(const struct sockaddr *addr, socklen_t addrlen) {
    if (bind(_sock, addr, addrlen) < 0) {
        std::cerr << "Error in bind: " << strerror(errno) << std::endl;
        throw SocketException();
    }
}

ssize_t UDPSocket::recvfrom_wrapper(void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    ssize_t recv_len = recvfrom(_sock, buf, len, flags, src_addr, addrlen);
    if(recv_len < 0) {
        if((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
            std::cerr << "Error in recvfrom: " << strerror(errno) << std::endl;
            throw SocketIOException();
        } //else: timeout (if set), will return (-1)
    }
    return recv_len;
}

void UDPSocket::sendto_wrapper(const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len) {
    ssize_t send_len = sendto(_sock, message, length, flags, dest_addr, dest_len);
    if(send_len < 0) {
        std::cerr << "Error in sendto: " << strerror(errno) << std::endl;
        throw SocketIOException();
    } else if(send_len < static_cast<ssize_t>(length)) {
        std::cerr << "Partial sendto" << std::endl;
        throw SocketIOException();
    }
}

void UDPSocket::write_wrapper(const void *buf, size_t count) {
    ssize_t write_len = write(_sock, buf, count);
    if(write_len < 0) {
        std::cerr << "Error in write: " << strerror(errno) << std::endl;
        throw SocketIOException();
    } else if(write_len < static_cast<ssize_t>(count)) {
        std::cerr << "Partial write" << std::endl;
        throw SocketIOException();
    }
}