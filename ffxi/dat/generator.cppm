module;

#include <cmath>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

module ffxi:dat.generator;

import :dat;
import :dat.d3m;
import glm;
import lotus;

namespace FFXI
{
class Keyframe : public DatChunk
{
public:
    Keyframe(char* name, uint8_t* buffer, size_t len);
    std::vector<std::pair<float, float>> intervals;
};

class Generator : public DatChunk
{
public:
#pragma pack(push, 2)
    struct GeneratorHeader
    {
        uint8_t flags1;
        uint8_t bone_point;
        uint8_t flags2;
        uint8_t flags3;
        uint32_t unknown1[3];
        float unknown2[16];
        uint32_t flags4;
        uint32_t unknown3[4];
        uint16_t unknown4;
        uint16_t interval;
        uint8_t occurences;
        uint8_t flags5;
        uint16_t unknown5;
        uint32_t flags6;
        uint32_t unknown_command_offset;
        uint32_t creation_command_offset;
        uint32_t tick_command_offset;
        uint32_t expiry_command_offset;
    };
#pragma pack(pop)

    Generator(char* name, uint8_t* buffer, size_t len);

    GeneratorHeader* header{nullptr};
    std::string id;
    uint16_t billboard{0};
    uint16_t pos_flags{0};
    glm::vec3 pos{0};
    uint8_t effect_type{0};
    uint16_t lifetime{0};

    // movement per frame
    glm::vec3 dpos{0};
    glm::vec3 dpos_fluctuation{0};
    glm::vec3 dpos_acceleration{0};
    float dpos_exp{1};

    // movement from/to origin
    float dpos_origin{0};

    // initial rotation
    glm::vec3 rot{0};
    glm::vec3 rot_fluctuation{0};

    // rotation per frame
    glm::vec3 drot{0};
    glm::vec3 drot_fluctuation{0};

    // scale
    glm::vec3 scale{0};
    glm::vec3 scale_fluctuation{0};
    float scale_all_fluctuation{0};

    // scale per frame
    glm::vec3 dscale{0};
    glm::vec3 dscale_fluctuation{0};

    // generation sphere
    float gen_radius_sphere{0};
    float gen_radius_sphere_fluctuation{0};

    uint32_t color{0};

    // generation cylinder
    float gen_radius{0};
    float gen_radius_fluctuation{0};
    glm::vec3 gen_multi{0};
    glm::vec2 gen_axis_rot{0};
    float gen_height{0};
    float gen_height_fluctuation{0};
    uint32_t rotations{0};

    // addition to rot every generation
    glm::vec3 gen_rot_add{0};

    // keyframe animation
    std::string kf_x_pos;
    std::string kf_y_pos;
    std::string kf_z_pos;

    std::string kf_x_rot;
    std::string kf_y_rot;
    std::string kf_z_rot;

    std::string kf_x_scale;
    std::string kf_y_scale;
    std::string kf_z_scale;

    std::string kf_r;
    std::string kf_g;
    std::string kf_b;
    std::string kf_a;

    std::string kf_u;
    std::string kf_v;

    glm::vec2 duv{};

    std::vector<D3M::Vertex> ring_vertices;
    std::vector<uint16_t> ring_indices;
    std::shared_ptr<lotus::Model> ring;
    std::string sub_generator;
    std::string end_generator;
};

Keyframe::Keyframe(char* _name, uint8_t* _buffer, size_t _len) : DatChunk(_name, _buffer, _len)
{
    intervals.resize(len / (sizeof(float) * 2));
    memcpy(intervals.data(), buffer, len);
}

Generator::Generator(char* _name, uint8_t* _buffer, size_t _len) : DatChunk(_name, _buffer, _len)
{
    header = (GeneratorHeader*)buffer;

    // data2: instructions to run at generation time
    uint8_t* data2 = buffer + header->creation_command_offset - 16;

    while (data2 < buffer + header->tick_command_offset - 16)
    {
        uint8_t data_type = *data2;
        uint8_t data_size = *(data2 + 1) & 0xF;
        data2 += 4;
        switch (data_type)
        {
        case 0x00:
            data2 = buffer + header->tick_command_offset - 16;
            break;
        case 0x01:
            // maybe these are just combined into one big uint32_t of flags
            billboard = *(uint16_t*)(data2);
            pos_flags = *(uint16_t*)(data2 + 2);
            id = std::string((char*)data2 + 8, 4);
            pos = *(glm::vec3*)(data2 + 16);
            effect_type = *(uint8_t*)(data2 + 29);
            lifetime = *(uint16_t*)(data2 + 30);
            break;
        case 0x02:
            dpos = *(glm::vec3*)(data2);
            break;
        case 0x03:
            dpos_fluctuation = *(glm::vec3*)(data2);
            break;
        case 0x06:
            gen_radius_sphere = *(float*)(data2);
            gen_radius_sphere_fluctuation = *(float*)(data2 + 4);
            break;
        case 0x07:
            break;
        case 0x08:
            dpos_origin = *(float*)(data2);
            break;
        case 0x09:
            rot = *(glm::vec3*)(data2);
            break;
        case 0x0a:
            rot_fluctuation = *(glm::vec3*)(data2);
            break;
        case 0x0b:
            drot = *(glm::vec3*)(data2);
            break;
        case 0x0c:
            drot_fluctuation = *(glm::vec3*)(data2);
            break;
        case 0x0f:
            scale = *(glm::vec3*)(data2);
            break;
        case 0x10:
            scale_fluctuation = *(glm::vec3*)(data2);
            break;
        case 0x11:
            scale_all_fluctuation = *(float*)(data2);
            break;
        case 0x12:
            dscale = *(glm::vec3*)(data2);
            break;
        case 0x13:
            dscale_fluctuation = *(glm::vec3*)(data2);
            break;
        case 0x16:
            color = *(uint32_t*)(data2);
            break;
        case 0x17:
            // color fluctuation?
            break;
        case 0x18:
            break;
        case 0x19:
            break;
        case 0x1a:
            break;
        case 0x1d:
            break;
        case 0x1e:
            // blend method?
            break;
        case 0x1f:
            gen_radius_fluctuation = *(float*)(data2);
            gen_radius = *(float*)(data2 + 4);
            gen_multi = *(glm::vec3*)(data2 + 8);
            gen_axis_rot = *(glm::vec2*)(data2 + 20);
            gen_height = *(float*)(data2 + 28);
            gen_height_fluctuation = *(float*)(data2 + 32);
            rotations = *(uint32_t*)(data2 + 40);
            break;
        case 0x21:
            kf_x_pos = std::string((char*)data2 + 4, 4);
            break;
        case 0x22:
            kf_y_pos = std::string((char*)data2 + 4, 4);
            break;
        case 0x23:
            kf_z_pos = std::string((char*)data2 + 4, 4);
            break;
        case 0x24:
            kf_z_rot = std::string((char*)data2 + 4, 4);
            break;
        case 0x25:
            kf_z_rot = std::string((char*)data2 + 4, 4);
            break;
        case 0x26:
            kf_z_rot = std::string((char*)data2 + 4, 4);
            break;
        case 0x27:
            kf_x_scale = std::string((char*)data2 + 4, 4);
            break;
        case 0x28:
            kf_y_scale = std::string((char*)data2 + 4, 4);
            break;
        case 0x29:
            kf_z_scale = std::string((char*)data2 + 4, 4);
            break;
        case 0x2a:
            kf_r = std::string((char*)data2 + 4, 4);
            break;
        case 0x2b:
            kf_g = std::string((char*)data2 + 4, 4);
            break;
        case 0x2c:
            kf_b = std::string((char*)data2 + 4, 4);
            break;
        case 0x2d:
            kf_a = std::string((char*)data2 + 4, 4);
            break;
        case 0x2e:
            kf_u = std::string((char*)data2 + 4, 4);
            break;
        case 0x2f:
            kf_v = std::string((char*)data2 + 4, 4);
            break;
        case 0x30:
            break;
        case 0x31:
            break;
        case 0x32:
            break;
        case 0x33:
            break;
        case 0x34:
            break;
        case 0x35:
            break;
        case 0x36:
            break;
        case 0x39:
            break;
        case 0x3a:
        {
            // generate a mesh (ring?)
            float* radii = (float*)(data2);
            uint32_t* colours = (uint32_t*)(data2 + 16);
            uint8_t segments = *(uint8_t*)(data2 + 32);
            uint8_t circles = 2 + *(uint8_t*)(data2 + 33);

            for (auto r = 0; r < circles; ++r)
            {
                for (auto v = 0; v < segments; ++v)
                {
                    D3M::Vertex vertex;
                    vertex.pos =
                        glm::vec3(std::cos((v * 2 * glm::pi<float>()) / segments) * radii[r], std::sin((v * 2 * glm::pi<float>()) / segments) * radii[r], 0.0);
                    vertex.normal = glm::vec3(0.0, 0.0, 1.0);
                    vertex.color = glm::vec4{((colours[r] & 0x0000FF)) / 255.0, ((colours[r] & 0x00FF00) >> 8) / 255.0, ((colours[r] & 0xFF0000) >> 16) / 255.0,
                                             ((colours[r] & 0xFF000000) >> 24) / 128.0};
                    vertex.uv = glm::vec2(0.0);
                    ring_vertices.push_back(vertex);
                }
            }
            for (auto r = 0; r < circles - 1; ++r)
            {
                uint16_t base = r * segments;
                for (auto v = 0; v < segments; ++v)
                {
                    ring_indices.push_back(base + v);
                    ring_indices.push_back(base + v + segments);
                    ring_indices.push_back(base + (v + 1) % segments);

                    ring_indices.push_back(base + v + segments);
                    ring_indices.push_back(base + (v + 1) % segments);
                    ring_indices.push_back(base + (v + 1) % segments + segments);
                }
            }
        }
        break;
        case 0x3b:
            gen_rot_add = *(glm::vec3*)(data2);
            break;
        case 0x3c:
            // another generator id
            break;
        case 0x3d:
            break;
        case 0x3e:
            break;
        case 0x3f:
            break;
        case 0x40:
            break;
        case 0x41:
            break;
        case 0x42:
            break;
        case 0x43:
            break;
        case 0x44:
        {
            sub_generator = std::string((char*)data2 + 4, 4);
        }
        break;
        case 0x45:
            break;
        case 0x47:
            break;
        case 0x49:
            break;
        case 0x4C:
            break;
        case 0x4F:
            break;
        case 0x53:
            break;
        case 0x54:
            break;
        case 0x55:
            break;
        case 0x56:
            break;
        case 0x58:
            break;
        case 0x59:
            break;
        case 0x5A:
            break;
        case 0x5B:
            break;
        case 0x60:
            break;
        case 0x61:
            break;
        case 0x62:
            break;
        case 0x63:
            break;
        case 0x67:
            break;
        case 0x6A:
            break;
        case 0x6C:
            break;
        case 0x72:
            break;
        case 0x78:
            break;
        case 0x82:
            break;
        case 0x87:
            break;
        case 0x8F:
            break;
        }

        data2 += (data_size - 1) * sizeof(uint32_t);
    }

    // data3: instructions to run on frame update
    uint8_t* data3 = buffer + header->tick_command_offset - 16;

    while (data3 < buffer + header->expiry_command_offset - 16)
    {
        uint8_t data_type = *data3;
        uint8_t data_size = *(data3 + 1) & 0xF;
        data3 += 4;

        switch (data_type)
        {
        case 0x00:
            data3 = buffer + header->expiry_command_offset - 16;
            break;

        case 0x03:
            dpos_acceleration = *(glm::vec3*)(data3);
            break;

        case 0x27:
            duv.x = *(float*)(data3);
            break;

        case 0x28:
            duv.y = *(float*)(data3);
            break;

        case 0x2C:
            dpos_exp = *(float*)(data3);
            break;
        }

        data3 += (data_size - 1) * sizeof(uint32_t);
    }

    // data4: instructions to run on particle end of life
    uint8_t* data4 = buffer + header->expiry_command_offset - 16;

    while (data4 < buffer + len)
    {
        uint8_t data_type = *data4;
        uint8_t data_size = *(data4 + 1) & 0xF;
        data4 += 4;

        switch (data_type)
        {
        case 0x00:
            data4 = buffer + len;
            break;

        case 0x01:
            end_generator = std::string((char*)data4 + 4, 4);
            break;
        }

        data4 += (data_size - 1) * sizeof(uint32_t);
    }
}
} // namespace FFXI
