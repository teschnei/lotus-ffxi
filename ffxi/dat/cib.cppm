module;

#include <cstdint>
#include <cstring>

module ffxi:dat.cib;

import :dat;

namespace FFXI
{
class Cib : public DatChunk
{
public:
    Cib(char* _name, uint8_t* _buffer, size_t _len) : DatChunk(_name, _buffer, _len) {}

    uint8_t unknown1;  // from skeleton
    uint8_t footstep1; // FootMat?
    uint8_t footstep2; // FootSize?
    uint8_t motion_index;
    uint8_t motion_option;
    uint8_t weapon_unknown;      // Shield?
    uint8_t weapon_unknown2;     // Constrain?
    uint8_t unknown2;            // probably always empty, maps to XiActorSkeleton's weapon_unknown2 for offhand
    uint8_t weapon_unknown3;     // never seen this populated, but decompiling says it's from a weapon
    uint8_t body_armour_unknown; // Waist?
    uint8_t scale;
    uint8_t unknown6; // float, default 1.000
    uint8_t unknown7; // float, default 1.000
    uint8_t unknown8; // float, default 1.000
    uint8_t motion_range_index;
};
} // namespace FFXI
