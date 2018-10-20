#ifndef SIKRADIO_AUDIO_H
#define SIKRADIO_AUDIO_H

#include <cstdint>
#include <cstddef>

class AudioPacket {
private:
    uint64_t _session_id;
    uint64_t _first_byte_num;
    std::byte *_audio_data;

    size_t _data_size;
public:
    AudioPacket(const std::byte* data, uint64_t session_id, uint64_t first_byte_num, size_t data_size);

    virtual ~AudioPacket();

    size_t write_to_buffer(std::byte* buffer) const;
};


#endif //SIKRADIO_AUDIO_H