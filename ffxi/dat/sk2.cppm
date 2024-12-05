module;

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <vector>

module ffxi:dat.sk2;

import :dat;
import glm;
import lotus;

namespace FFXI
{
class SK2 : public DatChunk
{
public:
#pragma pack(push, 2)
    struct Bone
    {
        uint8_t parent_index;
        uint8_t _pad;
        glm::quat rot;
        glm::vec3 trans;
    };

    struct GeneratorPoint
    {
        uint8_t bone_index;
        uint8_t _pad;
        float unknown[3];
        glm::vec3 offset;
    };
#pragma pack(pop)

    static constexpr size_t GeneratorPointMax = 120;

    SK2(char* _name, uint8_t* _buffer, size_t _len);
    std::vector<Bone> bones;
    std::array<GeneratorPoint, GeneratorPointMax> generator_points;
};

struct Skeleton
{
    uint16_t _pad;
    uint16_t bone_count;
};

SK2::SK2(char* _name, uint8_t* _buffer, size_t _len) : DatChunk(_name, _buffer, _len)
{
    Skeleton* skel = (Skeleton*)buffer;

    Bone* bone_buf = (Bone*)(buffer + sizeof(Skeleton));
    bones.resize(skel->bone_count);
    std::ranges::copy(std::span(bone_buf, bone_buf + skel->bone_count), bones.begin());

    GeneratorPoint* points = (GeneratorPoint*)(buffer + sizeof(Skeleton) + (sizeof(Bone) * skel->bone_count) + 4);
    std::ranges::copy(std::span(points, points + GeneratorPointMax), generator_points.begin());
}
} // namespace FFXI
