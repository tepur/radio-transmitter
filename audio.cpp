#include <endian.h>
#include <cstring>
#include "audio.h"

AudioPacket::AudioPacket(const std::byte* data, uint64_t session_id, uint64_t first_byte_num, size_t data_size) :
        _session_id(session_id), _first_byte_num(first_byte_num), _data_size(data_size) {
    _audio_data = new std::byte[data_size];
    for(size_t i = 0; i < data_size; i++) {
        _audio_data[i] = data[i];
    }
}

AudioPacket::~AudioPacket() {
    delete[] _audio_data;
}

size_t AudioPacket::write_to_buffer(std::byte* buffer) const {
    size_t buffer_pos = 0;

    uint64_t tmp = htobe64(_session_id);
    memcpy(buffer + buffer_pos, (std::byte*)&tmp, sizeof(tmp));
    buffer_pos += sizeof(_session_id);

    tmp = htobe64(_first_byte_num);
    memcpy(buffer + buffer_pos, (std::byte*)&tmp, sizeof(tmp));
    buffer_pos += sizeof(_first_byte_num);

    memcpy(buffer + buffer_pos, _audio_data, _data_size);
    buffer_pos += _data_size;

    return buffer_pos;
}