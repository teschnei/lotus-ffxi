module;

#include <algorithm>
#include <coroutine>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <optional>
#include <stb_image.h>
#include <unordered_map>
#include <vector>

module ffxi:dat.mzb;

import :dat;
import :dat.key_tables;
import glm;
import lotus;
import vulkan_hpp;

namespace FFXI
{
struct SMZBBlock100
{
    char id[16];
    float fTransX, fTransY, fTransZ;
    float fRotX, fRotY, fRotZ;
    float fScaleX, fScaleY, fScaleZ;
    float fa, fb, fc, fd; // 0, 10, 100, 1000
    uint32_t fe, ff, fg, fh, fi, fj, fk, fl;
};
static_assert(sizeof(SMZBBlock100) == 0x64);

struct CollisionMeshData
{
    std::vector<uint8_t> vertices;
    std::vector<uint8_t> normals;
    std::vector<uint16_t> indices;
    uint16_t flags{};
    uint32_t max_index;
    glm::vec3 bound_min{};
    glm::vec3 bound_max{};
};

class QuadTree
{
public:
    QuadTree(glm::vec3 pos1, glm::vec3 pos2) : pos1(pos1), pos2(pos2) {}

    std::vector<uint32_t> find(lotus::Component::CameraComponent::Frustum&) const;

    glm::vec3 pos1;
    glm::vec3 pos2;
    std::vector<uint32_t> nodes;
    std::vector<QuadTree> children;

private:
    void find_internal(lotus::Component::CameraComponent::Frustum&, std::vector<uint32_t>&) const;
    void get_nodes(std::vector<uint32_t>&) const;
};

class MZB : public DatChunk
{
public:
    MZB(char* _name, uint8_t* _buffer, size_t _len);

    static bool DecodeMZB(uint8_t* buffer, size_t max_len);
    std::vector<SMZBBlock100> vecMZB;
    std::optional<QuadTree> quadtree;
    std::vector<CollisionMeshData> meshes;
    std::vector<lotus::Model::TransformEntry> mesh_entries;
    std::unordered_map<float, std::vector<lotus::Model::TransformEntry>> water_entries;

    static lotus::Task<> LoadWaterModel(std::shared_ptr<lotus::Model>, lotus::Engine* engine, std::pair<glm::vec3, glm::vec3> bb);
    static lotus::Task<> LoadWaterTexture(std::shared_ptr<lotus::Texture>& texture, lotus::Engine* engine);

private:
    QuadTree parseQuadTree(uint8_t* buffer, uint32_t offset);
    uint32_t parseMesh(uint8_t* buffer, uint32_t offset);
    void parseGridEntry(uint8_t* buffer, uint32_t offset, int x, int y);
    void parseGridMesh(uint8_t* buffer, int x, int y, uint32_t vis_entry_offset, uint32_t geo_entry_offset);
    std::vector<uint32_t> vismap;
    std::unordered_map<uint32_t, uint32_t> mesh_map;
};

class CollisionLoader
{
public:
    static lotus::Task<> LoadModel(std::shared_ptr<lotus::Model>, lotus::Engine* engine, std::vector<CollisionMeshData>& meshes,
                                   std::vector<lotus::Model::TransformEntry>& entries);
};

struct SMZBHeader
{
    char id[4];
    uint32_t totalRecord100 : 24;
    uint32_t R100Flag : 8;
    uint32_t collisionMeshOffset;
    uint8_t gridWidth;
    uint8_t gridHeight;
    uint8_t bucketWidth;
    uint8_t bucketHeight;
    uint32_t quadtreeOffset;
    uint32_t objectOffsetEnd;
    uint32_t shortnamesOffset;
    int32_t unk5;
};
static_assert(sizeof(SMZBHeader) == 0x20);

struct SMZBBlock84
{
    char id[16];
    float fTransX, fTransY, fTransZ;
    float fRotX, fRotY, fRotZ;
    float fScaleX, fScaleY, fScaleZ;
    float fa, fb, fc, fd; // 0, 10, 100, 1000
    uint32_t i1, i2, i3, i4;
};

// observed in dat 116
struct SMZBBlock92b
{
    char id[16];
    float fTransX, fTransY, fTransZ;
    float fRotX, fRotY, fRotZ;
    float fScaleX, fScaleY, fScaleZ;
    float fa, fb, fc, fd; // 0, 10, 100, 1000
    uint32_t i1, i2, i3, i4, i5, i6;
};

enum class FrustumResult
{
    Inside,
    Outside,
    Intersect
};

FrustumResult isInFrustum(lotus::Component::CameraComponent::Frustum& frustum, glm::vec3 pos_min, glm::vec3 pos_max)
{
    FrustumResult result = FrustumResult::Inside;
    for (const auto& plane : {frustum.left, frustum.right, frustum.top, frustum.bottom, frustum.near, frustum.far})
    {
        // p: AABB corner furthest in the direction of plane normal, n: AABB corner furthest in the direction opposite
        // of plane normal
        glm::vec3 p = pos_min;
        glm::vec3 n = pos_max;

        if (plane.x >= 0)
        {
            p.x = pos_max.x;
            n.x = pos_min.x;
        }
        if (plane.y >= 0)
        {
            p.y = pos_max.y;
            n.y = pos_min.y;
        }
        if (plane.z >= 0)
        {
            p.z = pos_max.z;
            n.z = pos_min.z;
        }

        if (glm::dot(p, glm::vec3(plane)) + plane.w < 0.f)
        {
            return FrustumResult::Outside;
        }
        if (glm::dot(n, glm::vec3(plane)) + plane.w < 0.f)
        {
            result = FrustumResult::Intersect;
        }
    }
    return result;
}

void QuadTree::find_internal(lotus::Component::CameraComponent::Frustum& frustum, std::vector<uint32_t>& results) const
{
    auto result = isInFrustum(frustum, pos1, pos2);
    if (result == FrustumResult::Outside)
        return;
    else if (result == FrustumResult::Inside)
    {
        get_nodes(results);
    }
    else if (result == FrustumResult::Intersect)
    {
        results.insert(results.end(), nodes.begin(), nodes.end());
        for (const auto& child : children)
        {
            child.find_internal(frustum, results);
        }
    }
}

std::vector<uint32_t> QuadTree::find(lotus::Component::CameraComponent::Frustum& frustum) const
{
    std::vector<uint32_t> ret;
    find_internal(frustum, ret);
    return ret;
}

void QuadTree::get_nodes(std::vector<uint32_t>& results) const
{
    results.insert(results.end(), nodes.begin(), nodes.end());
    for (const auto& child : children)
    {
        child.get_nodes(results);
    }
}

MZB::MZB(char* _name, uint8_t* _buffer, size_t _len) : DatChunk(_name, _buffer, _len)
{
    SMZBHeader* header = (SMZBHeader*)buffer;

    for (size_t i = 0; i < header->totalRecord100; ++i)
    {
        vecMZB.push_back(((SMZBBlock100*)(buffer + sizeof(SMZBHeader)))[i]);
    }

    uint32_t mesh_count = *(uint32_t*)(buffer + header->collisionMeshOffset);
    uint32_t mesh_data = *(uint32_t*)(buffer + header->collisionMeshOffset + 0x04);

    uint32_t offset = mesh_data;
    for (uint32_t i = 0; i < mesh_count; ++i)
    {
        offset = parseMesh(buffer, offset);
    }

    uint32_t grid_offset = *(uint32_t*)(buffer + header->collisionMeshOffset + 0x10);

    for (int y = 0; y < header->gridHeight * header->bucketHeight / 4; ++y)
    {
        for (int x = 0; x < header->gridWidth * header->bucketWidth / 4; ++x)
        {
            uint32_t offsets = (y * header->gridWidth * header->bucketWidth / 4 + x) * 4;
            uint32_t entry_offset = *(uint32_t*)(buffer + grid_offset + offsets);
            if (entry_offset != 0)
            {
                parseGridEntry(buffer, entry_offset, x, y);
            }
        }
    }

    quadtree = parseQuadTree(buffer, header->quadtreeOffset);
}

bool MZB::DecodeMZB(uint8_t* buffer, size_t max_len)
{
    if (buffer[3] >= 0x1B)
    {
        uint32_t len = *(uint32_t*)buffer & 0x00FFFFFF;
        if (len > max_len)
            return false;

        uint32_t key = key_table[buffer[7] ^ 0xFF];
        int key_count = 0;
        uint32_t pos = 8;
        while (pos < len)
        {
            uint32_t xor_length = ((key >> 4) & 7) + 16;

            if ((key & 1) && (pos + xor_length < len))
            {
                for (uint32_t i = 0; i < xor_length; ++i)
                {
                    buffer[pos + i] ^= 0xFF;
                }
            }
            key += ++key_count;
            pos += xor_length;
        }

        uint32_t node_count = *(uint32_t*)(buffer + 4) & 0x00FFFFFF;

        SMZBBlock100* node = (SMZBBlock100*)(buffer + 32);

        for (uint32_t i = 0; i < node_count; ++i)
        {
            for (size_t j = 0; j < 16; ++j)
            {
                node->id[j] ^= 0x55;
            }
            ++node;
        }
    }
    return true;
}

QuadTree MZB::parseQuadTree(uint8_t* buffer, uint32_t offset)
{
    uint8_t* quad_base = buffer + offset;
    glm::vec3 pos1 = ((glm::vec3*)quad_base)[0];
    glm::vec3 pos2 = ((glm::vec3*)quad_base)[0];

    for (int i = 1; i < 8; ++i)
    {
        glm::vec3 bb = ((glm::vec3*)quad_base)[i];
        pos1 = glm::min(pos1, bb);
        pos2 = glm::max(pos2, bb);
    }

    QuadTree quadtree{pos1, pos2};

    uint32_t visibility_list_offset = *(uint32_t*)(quad_base + sizeof(glm::vec3) * 8);
    uint32_t visibility_list_count = *(uint32_t*)(quad_base + sizeof(glm::vec3) * 8 + sizeof(uint32_t));

    for (size_t i = 0; i < visibility_list_count; ++i)
    {
        uint32_t node = *(uint32_t*)(buffer + visibility_list_offset + sizeof(uint32_t) * i);
        quadtree.nodes.push_back(node);
    }

    for (int i = 0; i < 6; ++i)
    {
        uint32_t child_offset = *(uint32_t*)(quad_base + sizeof(glm::vec3) * 8 + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) * i);
        if (child_offset != 0)
        {
            quadtree.children.push_back(parseQuadTree(buffer, child_offset));
        }
    }

    return quadtree;
}

uint32_t MZB::parseMesh(uint8_t* buffer, uint32_t offset)
{
    uint32_t vertices_offset = *(uint32_t*)(buffer + offset + 0x00);
    uint32_t normals_offset = *(uint32_t*)(buffer + offset + 0x04);
    uint32_t triangles = *(uint32_t*)(buffer + offset + 0x08);
    uint16_t triangle_count = *(uint16_t*)(buffer + offset + 0x0c);
    uint16_t flags = *(uint16_t*)(buffer + offset + 0x0e);

    CollisionMeshData& mesh = meshes.emplace_back();

    glm::vec3* first_vertex = (glm::vec3*)(buffer + vertices_offset);
    mesh.bound_max = *first_vertex;
    mesh.bound_min = *first_vertex;

    for (auto buf = buffer + vertices_offset; buf < buffer + normals_offset; buf += sizeof(glm::vec3))
    {
        glm::vec3* vertex = (glm::vec3*)(buf);
        mesh.bound_max = glm::max(mesh.bound_max, *vertex);
        mesh.bound_min = glm::min(mesh.bound_min, *vertex);
    }

    mesh.vertices.resize(normals_offset - vertices_offset);
    memcpy(mesh.vertices.data(), buffer + vertices_offset, normals_offset - vertices_offset);

    mesh.normals.resize(triangles - normals_offset);
    memcpy(mesh.normals.data(), buffer + normals_offset, triangles - normals_offset);

    mesh.indices.reserve(triangle_count * 3);
    for (int i = 0; i < triangle_count; ++i)
    {
        mesh.indices.push_back((*(uint16_t*)(buffer + triangles + (i * 4 + 0) * 2)) & 0x3FFF);
        mesh.indices.push_back((*(uint16_t*)(buffer + triangles + (i * 4 + 1) * 2)) & 0x3FFF);
        mesh.indices.push_back((*(uint16_t*)(buffer + triangles + (i * 4 + 2) * 2)) & 0x3FFF);
    }
    mesh.max_index = *std::ranges::max_element(mesh.indices);

    mesh_map.insert({offset, meshes.size() - 1});

    return triangles + triangle_count * sizeof(uint16_t) * 4;
}

void MZB::parseGridEntry(uint8_t* buffer, uint32_t offset, int x, int y)
{
    uint32_t& pos = *(uint32_t*)(buffer + offset);
    uint32_t xx = (pos >> 14) & 0x1FF;
    uint32_t yy = (pos >> 23) & 0x1FF;
    uint32_t count = pos & 0x3FFF;
    pos = count;
    offset += 4;

    for (int i = 0; i < count; ++i)
    {
        uint32_t vis_entry_offset = *(uint32_t*)(buffer + offset);
        uint32_t geo_entry_offset = *(uint32_t*)(buffer + offset + 4);
        offset += 8;

        parseGridMesh(buffer, x, y, vis_entry_offset, geo_entry_offset);
    }
}

void MZB::parseGridMesh(uint8_t* buffer, int x, int y, uint32_t vis_entry_offset, uint32_t geo_entry_offset)
{
    glm::mat4& transform_matrix = *(glm::mat4*)(buffer + vis_entry_offset);
    // glm::mat4& transform_matrix2 = *(glm::mat4*)(buffer + vis_entry_offset + 64); //no idea what this is for
    // The top 6 bits and the bottom 4 bits are unused in this calculation, shift left to get the sign bit in the right
    // spot,
    //  then shift back plus an extra 4 to get rid of the lowest 4 bits
    float water_height = (((*(int32_t*)(buffer + vis_entry_offset + 164)) << 6) >> 10) / 1024.f;

    uint32_t mesh_offset = mesh_map[geo_entry_offset];

    mesh_entries.push_back({glm::transpose(transform_matrix), mesh_offset});

    if (water_height != 0)
    {
        // TODO: the mesh needs to be a flat plane, not a translated collision mesh
        glm::mat4 water_matrix = transform_matrix;
        water_matrix[3].y = water_height;
        water_entries[water_height].push_back({glm::transpose(water_matrix), mesh_offset});
    }
}

lotus::Task<> MZB::LoadWaterModel(std::shared_ptr<lotus::Model> model, lotus::Engine* engine, std::pair<glm::vec3, glm::vec3> bb)
{
    model->rendered = false;
    auto mesh = std::make_unique<lotus::Mesh>();

    std::shared_ptr<lotus::Buffer> material_buffer = engine->renderer->gpu->memory_manager->GetBuffer(
        lotus::Material::getMaterialBufferSize(engine),
        vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    std::shared_ptr<lotus::Texture> texture = co_await lotus::Texture::LoadTexture("water", LoadWaterTexture, engine);
    mesh->material = co_await lotus::Material::make_material(engine, material_buffer, 0, texture);

    struct WaterVertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    std::vector<WaterVertex> vertices = {{bb.first, {0.0, -1.0, 0.0}, {bb.first.x / 5, bb.first.z / 5}},
                                         {{bb.first.x, bb.first.y, bb.second.z}, {0.0, -1.0, 0.0}, {bb.first.x / 5, bb.second.z / 5}},
                                         {{bb.second.x, bb.first.y, bb.first.z}, {0.0, -1.0, 0.0}, {bb.second.x / 5, bb.first.z / 5}},
                                         {bb.second, {0.0, -1.0, 0.0}, {bb.second.x / 5, bb.second.z / 5}}};

    std::vector<uint16_t> indices = {2, 1, 0, 2, 1, 3};

    auto vertex_buffer_size = vertices.size() * sizeof(WaterVertex);
    auto index_buffer_size = indices.size() * sizeof(uint16_t);

    mesh->setMaxIndex(3);
    mesh->setVertexCount(vertices.size());
    mesh->setIndexCount(indices.size());

    model->meshes.push_back(std::move(mesh));
    model->lifetime = lotus::Lifetime::Long;

    std::vector<std::span<const std::byte>> vertices_vector = {std::as_bytes(std::span{vertices})};
    std::vector<std::span<const std::byte>> indices_vector = {std::as_bytes(std::span{indices})};

    co_await model->InitWork(engine, vertices_vector, indices_vector, sizeof(WaterVertex));
}

lotus::Task<> MZB::LoadWaterTexture(std::shared_ptr<lotus::Texture>& texture, lotus::Engine* engine)
{
    std::vector<std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>> pixels;
    int texWidth, texHeight, texChannels;
    stbi_uc* p = nullptr;
    for (auto& f : std::filesystem::directory_iterator("textures/water"))
    {
        if (auto path = f.path(); f.is_regular_file() && path.extension() == ".png")
        {
            auto p = stbi_load((const char*)path.u8string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if (!p)
            {
                throw std::runtime_error("failed to load texture image!");
            }
            pixels.push_back({p, &stbi_image_free});
        }
    }
    vk::DeviceSize imageSize = static_cast<uint64_t>(texWidth) * static_cast<uint64_t>(texHeight) * 4;

    texture->setWidth(texWidth);
    texture->setHeight(texHeight);

    std::vector<std::vector<uint8_t>> texture_datas;
    texture_datas.reserve(pixels.size());

    for (auto& p : pixels)
    {
        std::vector<uint8_t> pixel_data;
        pixel_data.resize(imageSize);
        memcpy(pixel_data.data(), p.get(), imageSize);
        texture_datas.push_back(std::move(pixel_data));
    }

    texture->image = engine->renderer->gpu->memory_manager->GetImage(
        texture->getWidth(), texture->getHeight(), vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, texture_datas.size());

    vk::ImageViewCreateInfo image_view_info;
    image_view_info.image = texture->image->image;
    image_view_info.viewType = vk::ImageViewType::e2DArray;
    image_view_info.format = vk::Format::eR8G8B8A8Unorm;
    image_view_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = texture_datas.size();

    texture->image_view = engine->renderer->gpu->device->createImageViewUnique(image_view_info, nullptr);

    vk::SamplerCreateInfo sampler_info = {};
    sampler_info.magFilter = vk::Filter::eLinear;
    sampler_info.minFilter = vk::Filter::eLinear;
    sampler_info.addressModeU = vk::SamplerAddressMode::eRepeat;
    sampler_info.addressModeV = vk::SamplerAddressMode::eRepeat;
    sampler_info.addressModeW = vk::SamplerAddressMode::eRepeat;
    sampler_info.anisotropyEnable = true;
    sampler_info.maxAnisotropy = 16;
    sampler_info.borderColor = vk::BorderColor::eIntOpaqueBlack;
    sampler_info.unnormalizedCoordinates = false;
    sampler_info.compareEnable = false;
    sampler_info.compareOp = vk::CompareOp::eAlways;
    sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;

    texture->sampler = engine->renderer->gpu->device->createSamplerUnique(sampler_info, nullptr);

    co_await texture->Init(engine, std::move(texture_datas));
}

lotus::Task<> CollisionLoader::LoadModel(std::shared_ptr<lotus::Model> model, lotus::Engine* engine, std::vector<CollisionMeshData>& collision_meshes,
                                         std::vector<lotus::Model::TransformEntry>& entries)
{
    model->rendered = false;

    std::vector<std::span<const std::byte>> vertices;
    std::vector<std::span<const std::byte>> indices;

    for (const auto& collision_mesh : collision_meshes)
    {
        vertices.push_back(std::as_bytes(std::span{collision_mesh.vertices}));
        indices.push_back(std::as_bytes(std::span{collision_mesh.indices}));
        auto mesh = std::make_unique<lotus::Mesh>();
        mesh->setIndexCount(static_cast<int>(collision_mesh.indices.size()));
        mesh->setMaxIndex(*std::ranges::max_element(collision_mesh.indices));
        model->meshes.push_back(std::move(mesh));
    }

    model->lifetime = lotus::Lifetime::Long;

    co_await model->InitWork(engine, vertices, indices, sizeof(float) * 3, std::move(entries));
}
} // namespace FFXI
