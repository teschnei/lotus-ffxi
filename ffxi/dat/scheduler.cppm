module;

#include <cstdint>
#include <cstring>
#include <utility>

module ffxi:dat.scheduler;

import :dat;

namespace FFXI
{
class Scheduler : public DatChunk
{
public:
#pragma pack(push, 2)
    struct SchedulerHeader
    {
        uint32_t unk0[4];
        uint32_t unk1;
        uint32_t unk2;
        uint32_t unk3;
        uint32_t unk4;
        uint32_t unk5;
        uint32_t unk6;
        uint32_t unk7;
        uint32_t unk8;
        uint32_t unk9;
        uint32_t unk10;
        uint32_t unk11;
        uint32_t unk12;
        uint32_t unk13;
        uint32_t unk14;
    };
#pragma pack(pop)
    Scheduler(char* name, uint8_t* buffer, size_t len);
    std::pair<uint8_t*, uint32_t> getStage(uint32_t stage);

    SchedulerHeader* header{nullptr};
    uint8_t* data{nullptr};
};

Scheduler::Scheduler(char* _name, uint8_t* _buffer, size_t _len) : DatChunk(_name, _buffer, _len)
{
    header = (SchedulerHeader*)buffer;

    data = buffer + sizeof(SchedulerHeader);
}

std::pair<uint8_t*, uint32_t> Scheduler::getStage(uint32_t stage)
{
    uint8_t* ret = data;
    uint32_t current_stage = 0;
    uint32_t frame = 0;
    while (ret < buffer + sizeof(SchedulerHeader) + len && current_stage < stage)
    {
        uint8_t type = *ret;
        uint8_t length = *(ret + 1) * sizeof(uint32_t);
        uint16_t delay = *(uint16_t*)(ret + 4);
        uint16_t duration = *(uint16_t*)(ret + 6);
        char* id = (char*)(ret + 8);

        if (len == 0)
            return {nullptr, 0};

        ret += length;
        frame += delay;
        current_stage++;
    }
    return {ret, frame};
}
} // namespace FFXI
