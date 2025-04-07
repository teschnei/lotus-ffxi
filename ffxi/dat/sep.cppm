module;

#include <cstdint>
#include <memory>

module ffxi:dat.sep;

import :audio;
import :dat;
import lotus;

namespace FFXI
{
class Audio;
class Sep : public DatChunk
{
public:
    Sep(char* name, uint8_t* buffer, size_t len);
    std::unique_ptr<lotus::AudioInstance> playSound(FFXI::Audio* audio);
};

Sep::Sep(char* _name, uint8_t* _buffer, size_t _len) : DatChunk(_name, _buffer, _len) {}

std::unique_ptr<lotus::AudioInstance> Sep::playSound(FFXI::Audio* audio)
{
    uint32_t id = *(uint32_t*)(buffer + 8);
    return audio->playSound(id);
}
} // namespace FFXI
