module;

#include "util.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

module ffxi:dat.os2;

import :dat;
import glm;
import vulkan_hpp;

namespace FFXI
{
class OS2 : public DatChunk
{
public:
    struct WeightingVertex
    {
        glm::vec3 pos;
        glm::vec3 norm;
        float weight;
        uint32_t bone_index;
        uint32_t mirror_axis;
        glm::vec2 uv;
    };

    struct WeightingVertexMirror
    {
        glm::vec3 pos;
        glm::vec3 norm;
        float weight;
        uint8_t bone_index;
        uint8_t bone_index_mirror;
        uint8_t mirror_axis;
    };

    // vertex format after transformation (animation_skin.comp)
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 norm;
        glm::vec2 uv;
    };

    struct Mesh
    {
        std::vector<std::pair<uint16_t, glm::vec2>> indices;
        std::string tex_name;
        float specular_exponent{};
        float specular_intensity{};
    };

    OS2(char* _name, uint8_t* _buffer, size_t _len);
    std::vector<Mesh> meshes;
    std::vector<std::pair<WeightingVertexMirror, WeightingVertexMirror>> vertices;
    bool mirror;
};

#pragma pack(push, 2)
// from noesis
struct MeshHeader
{
    uint16_t mUnknown1;
    uint16_t mVertAndBoneRefFlag;
    uint16_t mMirror;

    uint32_t mDrawDataOfs;
    uint16_t mDrawDataSize;

    uint32_t mBoneRefOfs;
    uint16_t mBoneRefCount;

    uint32_t mWeightedVertCountOfs;
    uint16_t mMaxWeightsPerVertex;

    uint32_t mWeightDataOfs;
    uint16_t mWeightDataCount;

    uint32_t mVertOfs;
    uint16_t mVertDataSize;

    uint32_t mUnknown2Ofs;
    uint16_t mUnknown3;
    uint16_t mUnknown4;
    uint16_t mUnknown5;

    uint32_t mUnknown6Ofs;
    uint16_t mUnknown7;

    uint32_t mUnknown8;
    uint32_t mUnknown9;
    uint16_t mUnknown10;
    uint16_t mUnknown11;
};

struct DrawStateHeader
{
    unsigned char a[4];
    float b[2];
    uint32_t c;
    float d[4];
    uint32_t e;
    float specular_exponent;
    float specular_intensity;
};

struct TriangleList
{
    uint16_t indices[3];
    glm::vec2 uvs[3];
};

struct TriangleStrip
{
    uint16_t index;
    glm::vec2 uv;
};

struct BoneIndices
{
    uint16_t bone_index1 : 7;
    uint16_t bone_index2 : 7;
    uint16_t mirror_axis : 2;
};
#pragma pack(pop)

OS2::OS2(char* _name, uint8_t* _buffer, size_t _len) : DatChunk(_name, _buffer, _len)
{
    MeshHeader* header = (MeshHeader*)buffer;

    mirror = header->mMirror > 0;

    uint8_t* draw_cmds = ((uint8_t*)header) + header->mDrawDataOfs * 2;
    uint8_t* draw_cmd_end = ((uint8_t*)header) + header->mDrawDataOfs * 2 + header->mDrawDataSize * 2;

    bool normals = (header->mVertAndBoneRefFlag & 0x7F) == 0;

    while (draw_cmds < draw_cmd_end)
    {
        uint16_t cmd = *(uint16_t*)(draw_cmds);
        draw_cmds += sizeof(uint16_t);

        // these probably map pretty well to opengl cmds (maybe even vulkan commands) but we need a flat VB anyways for
        // acceleration structures
        switch (cmd)
        {
        // draw state
        case 0x8010:
        {
            DrawStateHeader* draw_state = (DrawStateHeader*)draw_cmds;
            draw_cmds += 44;
            meshes.push_back({});
            meshes.back().specular_exponent = draw_state->specular_exponent;
            meshes.back().specular_intensity = draw_state->specular_intensity;
        }
        break;
        // set material
        case 0x8000:
        {
            if (!meshes.back().tex_name.empty())
                DEBUG_BREAK();
            meshes.back().tex_name = std::string{(char*)draw_cmds, 16};
            draw_cmds += 16;
        }
        break;
        // triangle list
        case 0x0054:
        {
            uint16_t count = *(uint16_t*)draw_cmds;
            draw_cmds += sizeof(uint16_t);

            for (size_t j = 0; j < count; ++j)
            {
                TriangleList* list = (TriangleList*)draw_cmds;
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    meshes.back().indices.push_back({list->indices[vertex], list->uvs[vertex]});
                }
                draw_cmds += sizeof(TriangleList);
            }
        }
        break;
        // triangle strip
        case 0x5453:
        {
            uint16_t count = *(uint16_t*)draw_cmds;
            draw_cmds += sizeof(uint16_t);
            TriangleList* list = (TriangleList*)draw_cmds;
            TriangleStrip prev{};
            TriangleStrip prev2{};
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                meshes.back().indices.push_back({list->indices[vertex], list->uvs[vertex]});
            }
            prev.index = list->indices[2];
            prev.uv = list->uvs[2];
            prev2.index = list->indices[1];
            prev2.uv = list->uvs[1];
            draw_cmds += sizeof(TriangleList);

            for (size_t j = 0; j < count - 1; ++j)
            {
                TriangleStrip* strip = (TriangleStrip*)draw_cmds;
                meshes.back().indices.push_back({prev2.index, prev2.uv});
                meshes.back().indices.push_back({prev.index, prev.uv});
                meshes.back().indices.push_back({strip->index, strip->uv});
                prev2 = prev;
                prev = *strip;
                draw_cmds += sizeof(TriangleStrip);
            }
        }
        break;
        // unknown
        case 0x4353:
        {
            uint16_t count = *(uint16_t*)draw_cmds;

            draw_cmds += 8 + sizeof(uint16_t) * count;
        }
        break;
        // unknown
        case 0x0043:
        {
            uint16_t count = *(uint16_t*)draw_cmds;

            draw_cmds += count * 10;
        }
        break;
        // end
        case 0xFFFF:
        {
        }
        break;
        default:
        {
            DEBUG_BREAK();
        }
        }
    }

    std::vector<uint16_t> bone_table;
    uint16_t* bone_table_buffer = (uint16_t*)header + header->mBoneRefOfs;
    for (int i = 0; i < header->mBoneRefCount; ++i)
    {
        bone_table.push_back(bone_table_buffer[i]);
    }

    float* vertex_buffer = (float*)((uint16_t*)header + header->mVertOfs);
    BoneIndices* bone_indices = (BoneIndices*)((uint16_t*)header + header->mWeightDataOfs);

    size_t one_weight_count = -1;
    size_t two_weight_count = 0;
    if (header->mWeightedVertCountOfs > 0)
    {
        one_weight_count = ((uint16_t*)header + header->mWeightedVertCountOfs)[0];
        two_weight_count = ((uint16_t*)header + header->mWeightedVertCountOfs)[1];
    }
    else
    {
        DEBUG_BREAK();
    }

    bool use_bone_table = header->mVertAndBoneRefFlag & 0x80;

    for (size_t i = 0; i < one_weight_count; ++i)
    {
        WeightingVertexMirror vertex{};
        vertex.pos = {vertex_buffer[0], vertex_buffer[1], vertex_buffer[2]};
        vertex_buffer += 3;
        if (normals)
        {
            vertex.norm = {vertex_buffer[0], vertex_buffer[1], vertex_buffer[2]};
            vertex_buffer += 3;
        }
        if (use_bone_table)
        {
            vertex.bone_index = bone_table[bone_indices->bone_index1];
            vertex.bone_index_mirror = bone_table[bone_indices->bone_index2];
        }
        else
        {
            vertex.bone_index = bone_indices->bone_index1;
            vertex.bone_index_mirror = bone_indices->bone_index2;
        }
        vertex.mirror_axis = bone_indices->mirror_axis;
        vertex.weight = 1.f;
        bone_indices += 2;
        vertices.emplace_back(vertex, WeightingVertexMirror{});
    }
    for (size_t i = 0; i < two_weight_count; ++i)
    {
        WeightingVertexMirror vertex1{};
        WeightingVertexMirror vertex2{};
        vertex1.pos = {vertex_buffer[0], vertex_buffer[2], vertex_buffer[4]};
        vertex2.pos = {vertex_buffer[1], vertex_buffer[3], vertex_buffer[5]};
        vertex_buffer += 6;
        vertex1.weight = vertex_buffer[0];
        vertex2.weight = vertex_buffer[1];
        vertex_buffer += 2;
        if (normals)
        {
            vertex1.norm = {vertex_buffer[0], vertex_buffer[2], vertex_buffer[4]};
            vertex2.norm = {vertex_buffer[1], vertex_buffer[3], vertex_buffer[5]};
            vertex_buffer += 6;
        }
        if (use_bone_table)
        {
            vertex1.bone_index = bone_table[bone_indices->bone_index1];
            vertex1.bone_index_mirror = bone_table[bone_indices->bone_index2];
            vertex1.mirror_axis = bone_indices->mirror_axis;
            bone_indices++;
            vertex2.bone_index = bone_table[bone_indices->bone_index1];
            vertex2.bone_index_mirror = bone_table[bone_indices->bone_index2];
            vertex2.mirror_axis = bone_indices->mirror_axis;
            bone_indices++;
        }
        else
        {
            vertex1.bone_index = bone_indices->bone_index1;
            vertex1.bone_index_mirror = bone_indices->bone_index2;
            vertex1.mirror_axis = bone_indices->mirror_axis;
            bone_indices++;
            vertex2.bone_index = bone_indices->bone_index1;
            vertex2.bone_index_mirror = bone_indices->bone_index2;
            vertex2.mirror_axis = bone_indices->mirror_axis;
            bone_indices++;
        }
        vertices.emplace_back(vertex1, vertex2);
    }
}
} // namespace FFXI
