#ifndef SIKRADIO_SOCKET_H
#define SIKRADIO_SOCKET_H

#include <exception>

class SocketException : std::exception {};
class SocketIOException : SocketException {};

class UDPSocket {
private:
    int _sock = -1;
public:
    virtual ~UDPSocket();

    void socket_wrapper(int domain, int type, int protocol);

    void setsockopt_wrapper(int level, int option_name, const void *option_value, socklen_t option_len);

    static void inet_aton_wrapper(const char *cp, struct in_addr *inp);

    void connect_wrapper(const struct sockaddr *address, socklen_t address_len);

    void bind_wrapper(const struct sockaddr *addr, socklen_t addrlen);

    ssize_t recvfrom_wrapper(void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

    void sendto_wrapper(const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);

    void write_wrapper(const void *buf, size_t count);
};

#endif //SIKRADIO_SOCKET_H
