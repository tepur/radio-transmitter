#include <iostream>
#include <chrono>
#include <algorithm>
#include <queue>
#include <list>
#include "transmitter.h"
#include "c_array_wrapper.h"

void Transmitter::rexmit_packets(std::set<uint64_t> packet_first_byte_nums) {
    CArrayWrapper<std::byte> queue_fragment(_psize);

    for(auto first_byte_num : packet_first_byte_nums) {
        _last_bytes_queue.mutex.lock();
        long queue_start_index = (_last_bytes_queue.val->size() - (_n_of_bytes_read - first_byte_num));

        if((first_byte_num % _psize != 0) || (queue_start_index < 0) ||
                ((queue_start_index + _psize) >= (_last_bytes_queue.val->size()))) {
            _last_bytes_queue.mutex.unlock();
            continue;
        }

        for(size_t i = static_cast<size_t>(queue_start_index), j = 0; j < _psize; i++, j++) {
            queue_fragment.set(j, _last_bytes_queue.val->at(i));
        }
        _last_bytes_queue.mutex.unlock();

        AudioPacket tmp(queue_fragment.get_raw_array(), _default_session_id, first_byte_num, _psize);
        try {
            send_packet_to_mcast(tmp);
        } catch (SocketIOException& exc) {
            std::cerr << "Error in sending a rexmit packet, try to continue working" << std::endl;
        }
    }
}

void Transmitter::set_listener_socket(UDPSocket &sock) {
    sock.socket_wrapper(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in listen_address;
    listen_address.sin_family = AF_INET;
    listen_address.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_address.sin_port = htons(_ctrl_port);

    sock.bind_wrapper((struct sockaddr *)&listen_address, sizeof listen_address);
}

void Transmitter::set_socket_timeout(UDPSocket& sock, const std::chrono::system_clock::time_point& start_time,
                                     const rtime_type& time_for_work) {
    std::chrono::duration<long, std::micro> time_remaining = std::chrono::duration_cast<std::chrono::microseconds>(
            (time_for_work - (std::chrono::system_clock::now() - start_time)));

    struct timeval recv_time_left;

    if(time_remaining.count() < 0) {
        recv_time_left.tv_sec = recv_time_left.tv_usec = 0;
    } else {
        recv_time_left.tv_usec = (time_remaining.count() % 1000000);
        recv_time_left.tv_sec = (time_remaining.count() / 1000000);
    }

    sock.setsockopt_wrapper(SOL_SOCKET, SO_RCVTIMEO, &recv_time_left, sizeof(struct timeval));
}

void erase_ended_tasks(std::list<std::future<void> > &tasks) {
    std::list<std::future<void> >::iterator task_it = tasks.begin(), task_to_erase;
    while(task_it != tasks.end()) {
        if(task_it->wait_for(std::chrono::microseconds(0)) == std::future_status::ready) {
            task_to_erase = task_it;
            ++task_it;
            tasks.erase(task_to_erase);
        } else {
            ++task_it;
        }
    }
}

void wait_for_tasks(std::list<std::future<void> >& tasks) {
    while(!tasks.empty()) {
        tasks.front().get();
        tasks.pop_front();
    }
}

std::vector<uint64_t> parse_packet_numbers(const std::string& str) {
    std::vector<uint64_t> packet_numbers;
    std::string::size_type packet_number_start_pos = str.find_first_of(' ');
    if(packet_number_start_pos != std::string::npos) {
        packet_number_start_pos++;
    }

    std::string::size_type packet_number_end_pos;
    uint64_t packet_number;
    while(packet_number_start_pos < str.length()) {
        packet_number_end_pos = str.find(',', packet_number_start_pos);
        packet_number = std::stoull(str.substr(packet_number_start_pos,
                                               packet_number_end_pos - packet_number_start_pos));
        packet_numbers.push_back(packet_number);

        packet_number_start_pos = packet_number_end_pos;
        if(packet_number_start_pos != std::string::npos) {
            packet_number_start_pos++;
        }
    }
    return packet_numbers;
}

void Transmitter::listen_to_receivers() {
    static constexpr const int LOOKUP_ANSWER_MAX_LEN = 200;
    static constexpr const int MAX_UDP_PACKET_SIZE = 65540;

    static constexpr const char* lookup_message = "ZERO_SEVEN_COME_IN\n";

    static const std::regex rexmit_message_regex =
            std::regex("LOUDER_PLEASE ((0)|([1-9]([0-9]*)))(,((0)|([1-9]([0-9]*))))*\n");

    UDPSocket listen_sock;
    try {
        set_listener_socket(listen_sock);
    } catch (SocketException& exc) {
        std::cerr << "Error: cannot initialize listen socket" << std::endl;
        return;
    }

    std::chrono::system_clock::time_point start_time;
    ssize_t rcv_len;
    int flags = 0, sflags = 0;
    struct sockaddr_in receiver_address;
    socklen_t receiver_address_len = (socklen_t) sizeof(receiver_address);

    char buffer[MAX_UDP_PACKET_SIZE];
    char lookup_answer[LOOKUP_ANSWER_MAX_LEN];

    sprintf(lookup_answer, "BOREWICZ_HERE %s %d %s\n", _mcast_addr.c_str(), _data_port, _station_name.c_str());
    size_t lookup_answer_len = strlen(lookup_answer);

    std::string received_message;
    std::set<uint64_t> packets_to_rexmit;

    std::list<std::future<void> > senders;

    while(_transmitter_is_on) {
        start_time = std::chrono::system_clock::now();

        while(any_time_left(start_time, _rtime)) {
            try {
                set_socket_timeout(listen_sock, start_time, _rtime);
                rcv_len = listen_sock.recvfrom_wrapper(buffer, sizeof(buffer), flags,
                                                       (struct sockaddr *) &receiver_address, &receiver_address_len);

                if(rcv_len < 0) { //timeout
                    continue;
                }

                buffer[rcv_len] = '\0';
                received_message = buffer;

                if (received_message == lookup_message) {
                    listen_sock.sendto_wrapper(lookup_answer, (size_t) lookup_answer_len, sflags,
                                               (struct sockaddr *) &receiver_address, receiver_address_len);
                } else if (std::regex_match(received_message, rexmit_message_regex)) {
                    for (auto elem : parse_packet_numbers(received_message)) {
                        packets_to_rexmit.insert(elem);
                    }
                } else {
                    std::clog << "Received wrong message" << std::endl;
                }
            } catch (SocketIOException &exc) {
                std::cerr << "Error in IO operation, try to continue" << std::endl;
            } catch (SocketException& exc) {
                std::cerr << "Problem with socket" << std::endl;
                wait_for_tasks(senders);
                return;
            }
        }
        senders.push_back(std::async(&Transmitter::rexmit_packets, this, std::move(packets_to_rexmit)));
        packets_to_rexmit.clear();

        erase_ended_tasks(senders);
    }
    wait_for_tasks(senders);
}

void Transmitter::run() {
    if(!_transmitter_is_on) {
        try {
            init();
        } catch (SocketException& exc) {
            std::cerr << "Error: cannot initialize server" << std::endl;
            return;
        }
    }

    _rexmit_task = std::async(&Transmitter::listen_to_receivers, this);

    _n_of_bytes_read = 0;

    CArrayWrapper<std::byte> buffer(_psize);

    while ((fread(buffer.get_raw_array(), sizeof(std::byte), _psize, stdin)) == _psize) {
        AudioPacket current_packet(buffer.get_raw_array(), _default_session_id, _n_of_bytes_read, _psize);
        _last_bytes_queue.mutex.lock();
        for(size_t i = 0; i < _psize; i++) {
            _last_bytes_queue.val->push(buffer[i]);
        }
        _last_bytes_queue.mutex.unlock();
        _n_of_bytes_read += _psize;
        try {
            send_packet_to_mcast(current_packet);
        } catch (SocketIOException& exc) {
            std::cerr << "Error in sending a packet, try to continue working" << std::endl;
        }
    }

    _transmitter_is_on = false;
    _rexmit_task.get();
}

void Transmitter::init() {
    init_server();

    _last_bytes_queue.val = new FixedSizeFIFODeque<std::byte>(_fsize);
    _send_buffer.val = new std::byte[whole_packet_size()];

    _transmitter_is_on = true;
}

void Transmitter::init_server() {
    static const int TTL_VALUE = 32;
    _send_sock.socket_wrapper(AF_INET, SOCK_DGRAM, 0);

    int optval = 1;
    _send_sock.setsockopt_wrapper(SOL_SOCKET, SO_BROADCAST, (void *) &optval, sizeof optval);

    optval = TTL_VALUE;
    _send_sock.setsockopt_wrapper(IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof optval);

    struct sockaddr_in remote_address;
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(_data_port);
    _send_sock.inet_aton_wrapper(_mcast_addr.c_str(), &remote_address.sin_addr);

    _send_sock.connect_wrapper((struct sockaddr *) &remote_address, sizeof remote_address);
}

void Transmitter::send_packet_to_mcast(const AudioPacket &packet) {
    std::lock_guard<std::mutex> lock(_send_buffer.mutex);
    size_t packet_length = packet.write_to_buffer(_send_buffer.val);
    _send_sock.write_wrapper(_send_buffer.val, packet_length);
}