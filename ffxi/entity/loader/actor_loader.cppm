module;

#include <algorithm>
#include <atomic>
#include <coroutine>
#include <cstring>
#include <latch>
#include <map>
#include <memory>
#include <span>
#include <vector>

module ffxi:entity.loader.actor;

import :dat.os2;
import :entity.actor;
import lotus;
import glm;
import vulkan_hpp;

class FFXIActorLoader
{
public:
    static lotus::Task<> LoadModel(std::shared_ptr<lotus::Model>, lotus::Engine* engine, std::span<FFXI::OS2* const> os2s);

private:
    static void InitPipeline(lotus::Engine*);
    static inline vk::Pipeline pipeline;
    static inline vk::Pipeline pipeline_shadowmap;
    static inline std::latch pipeline_latch{1};
    static inline std::atomic_flag pipeline_flag;
};

std::vector<vk::VertexInputBindingDescription> getBindingDescriptions()
{
    return std::vector<vk::VertexInputBindingDescription>{{
                                                              .binding = 0,
                                                              .stride = sizeof(FFXI::OS2::Vertex),
                                                              .inputRate = vk::VertexInputRate::eVertex,
                                                          },
                                                          {
                                                              .binding = 1,
                                                              .stride = sizeof(FFXI::OS2::Vertex),
                                                              .inputRate = vk::VertexInputRate::eVertex,
                                                          }};
}

std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions()
{
    return std::vector<vk::VertexInputAttributeDescription>{
        {
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(FFXI::OS2::Vertex, pos),
        },
        {
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(FFXI::OS2::Vertex, norm),
        },
        {
            .location = 2,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(FFXI::OS2::Vertex, uv),
        },
        {
            .location = 3,
            .binding = 1,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(FFXI::OS2::Vertex, pos),
        },
    };
}

lotus::Task<> FFXIActorLoader::LoadModel(std::shared_ptr<lotus::Model> model, lotus::Engine* engine, std::span<FFXI::OS2* const> os2s)
{
    if (!pipeline_flag.test_and_set())
    {
        InitPipeline(engine);
        pipeline_latch.count_down();
    }
    pipeline_latch.wait();
    model->light_offset = 0;
    std::vector<std::vector<FFXI::OS2::WeightingVertex>> vertices;
    std::vector<std::vector<uint16_t>> indices;
    std::vector<std::span<const std::byte>> vertex_bytes;
    std::vector<std::span<const std::byte>> index_bytes;
    std::map<lotus::Mesh*, lotus::WorkerTask<std::shared_ptr<lotus::Material>>> material_map;

    for (const auto& os2 : os2s)
    {
        if (os2->meshes.size() > 0)
        {
            std::shared_ptr<lotus::Buffer> material_buffer = engine->renderer->gpu->memory_manager->GetBuffer(
                lotus::Material::getMaterialBufferSize(engine) * os2->meshes.size(),
                vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress,
                vk::MemoryPropertyFlagBits::eDeviceLocal);
            uint32_t material_buffer_offset = 0;
            for (const auto& os2_mesh : os2->meshes)
            {
                auto mesh = std::make_unique<lotus::Mesh>();
                mesh->has_transparency = true;

                std::vector<FFXI::OS2::WeightingVertex> os2_vertices;
                std::vector<uint16_t> mesh_indices;
                std::shared_ptr<lotus::Texture> texture = lotus::Texture::getTexture(os2_mesh.tex_name);
                if (!texture)
                    texture = lotus::Texture::getTexture("default");
                float roughness = os2_mesh.specular_exponent / 128;
                float ior = os2_mesh.specular_intensity == 0 ? 0 : os2_mesh.specular_intensity + 1;
                material_map.insert(std::make_pair(
                    mesh.get(), lotus::Material::make_material(engine, material_buffer, material_buffer_offset, texture, 0, glm::vec2(roughness), ior)));
                material_buffer_offset += lotus::Material::getMaterialBufferSize(engine);

                int passes = os2->mirror ? 2 : 1;
                for (int i = 0; i < passes; ++i)
                {
                    for (auto [index, uv] : os2_mesh.indices)
                    {
                        const auto& vert = os2->vertices[index];
                        FFXI::OS2::WeightingVertex vertex;
                        vertex.uv = uv;
                        vertex.pos = vert.first.pos;
                        vertex.norm = vert.first.norm;
                        vertex.weight = vert.first.weight;
                        if (i == 0)
                        {
                            vertex.bone_index = vert.first.bone_index;
                            vertex.mirror_axis = 0;
                        }
                        else
                        {
                            vertex.bone_index = vert.first.bone_index_mirror;
                            vertex.mirror_axis = vert.first.mirror_axis;
                        }
                        os2_vertices.push_back(vertex);
                        vertex.pos = vert.second.pos;
                        vertex.norm = vert.second.norm;
                        vertex.weight = vert.second.weight;
                        if (i == 0)
                        {
                            vertex.bone_index = vert.second.bone_index;
                            vertex.mirror_axis = 0;
                        }
                        else
                        {
                            vertex.bone_index = vert.second.bone_index_mirror;
                            vertex.mirror_axis = vert.second.mirror_axis;
                        }
                        os2_vertices.push_back(vertex);

                        mesh_indices.push_back((uint16_t)mesh_indices.size());
                    }
                }

                mesh->setIndexCount(mesh_indices.size());
                mesh->setVertexCount(os2_vertices.size());
                mesh->setMaxIndex(*std::ranges::max_element(mesh_indices));
                mesh->setVertexInputAttributeDescription(getAttributeDescriptions(), sizeof(FFXI::OS2::WeightingVertex));
                mesh->setVertexInputBindingDescription(getBindingDescriptions());
                mesh->pipelines.push_back(pipeline);
                mesh->pipelines.push_back(pipeline_shadowmap);

                vertices.push_back(std::move(os2_vertices));
                indices.push_back(std::move(mesh_indices));

                vertex_bytes.push_back(std::as_bytes(std::span{vertices.back()}));
                index_bytes.push_back(std::as_bytes(std::span{indices.back()}));

                model->meshes.push_back(std::move(mesh));
            }
        }
    }
    model->lifetime = lotus::Lifetime::Short;
    model->weighted = true;

    for (auto& [mesh, task] : material_map)
    {
        mesh->material = co_await task;
    }

    co_await model->InitWork(engine, vertex_bytes, index_bytes, sizeof(FFXI::OS2::WeightingVertex));
}

void FFXIActorLoader::InitPipeline(lotus::Engine* engine)
{
    auto shader_module = engine->renderer->getShader("shaders/sk2.spv");

    vk::PipelineShaderStageCreateInfo vert_shader_stage_info;
    vert_shader_stage_info.stage = vk::ShaderStageFlagBits::eVertex;
    vert_shader_stage_info.module = *shader_module;
    vert_shader_stage_info.pName = "Vertex";

    vk::PipelineShaderStageCreateInfo frag_shader_stage_info;
    frag_shader_stage_info.stage = vk::ShaderStageFlagBits::eFragment;
    frag_shader_stage_info.module = *shader_module;
    frag_shader_stage_info.pName = "Fragment";

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vert_shader_stage_info, frag_shader_stage_info};

    vk::PipelineVertexInputStateCreateInfo main_vertex_input_info;

    auto main_binding_descriptions = getBindingDescriptions();
    auto main_attribute_descriptions = getAttributeDescriptions();

    main_vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(main_binding_descriptions.size());
    main_vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(main_attribute_descriptions.size());
    main_vertex_input_info.pVertexBindingDescriptions = main_binding_descriptions.data();
    main_vertex_input_info.pVertexAttributeDescriptions = main_attribute_descriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.topology = vk::PrimitiveTopology::eTriangleList;
    input_assembly.primitiveRestartEnable = false;

    vk::PipelineViewportStateCreateInfo viewport_state;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.depthClampEnable = false;
    rasterizer.rasterizerDiscardEnable = false;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eNone;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = false;

    vk::PipelineMultisampleStateCreateInfo multisampling;
    multisampling.sampleShadingEnable = false;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineDepthStencilStateCreateInfo depth_stencil;
    depth_stencil.depthTestEnable = true;
    depth_stencil.depthWriteEnable = true;
    depth_stencil.depthCompareOp = vk::CompareOp::eLessOrEqual;
    depth_stencil.depthBoundsTestEnable = false;
    depth_stencil.stencilTestEnable = false;

    vk::PipelineColorBlendAttachmentState color_blend_attachment;
    color_blend_attachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    color_blend_attachment.blendEnable = false;
    color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;
    color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
    color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcColor;
    color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcColor;
    color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;

    vk::PipelineColorBlendAttachmentState color_blend_attachment_accumulation = color_blend_attachment;
    color_blend_attachment_accumulation.blendEnable = true;
    color_blend_attachment_accumulation.srcColorBlendFactor = vk::BlendFactor::eOne;
    color_blend_attachment_accumulation.dstColorBlendFactor = vk::BlendFactor::eOne;
    color_blend_attachment_accumulation.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    color_blend_attachment_accumulation.dstAlphaBlendFactor = vk::BlendFactor::eOne;
    vk::PipelineColorBlendAttachmentState color_blend_attachment_revealage = color_blend_attachment;
    color_blend_attachment_revealage.blendEnable = true;
    color_blend_attachment_revealage.srcColorBlendFactor = vk::BlendFactor::eZero;
    color_blend_attachment_revealage.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcColor;
    color_blend_attachment_revealage.srcAlphaBlendFactor = vk::BlendFactor::eZero;
    color_blend_attachment_revealage.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcColor;

    std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states(7, color_blend_attachment);
    std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states_subpass1{color_blend_attachment_accumulation,
                                                                                              color_blend_attachment_revealage};

    vk::PipelineColorBlendStateCreateInfo color_blending;
    color_blending.logicOpEnable = false;
    color_blending.logicOp = vk::LogicOp::eCopy;
    color_blending.attachmentCount = static_cast<uint32_t>(color_blend_attachment_states.size());
    color_blending.pAttachments = color_blend_attachment_states.data();
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    vk::PipelineColorBlendStateCreateInfo color_blending_subpass1 = color_blending;
    color_blending_subpass1.attachmentCount = static_cast<uint32_t>(color_blend_attachment_states_subpass1.size());
    color_blending_subpass1.pAttachments = color_blend_attachment_states_subpass1.data();

    std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eScissor, vk::DynamicState::eViewport};
    vk::PipelineDynamicStateCreateInfo dynamic_state_ci{.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
                                                        .pDynamicStates = dynamic_states.data()};

    vk::GraphicsPipelineCreateInfo pipeline_info;
    pipeline_info.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipeline_info.pStages = shaderStages.data();
    pipeline_info.pVertexInputState = &main_vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state_ci;
    pipeline_info.basePipelineHandle = nullptr;

    pipeline = engine->renderer->createGraphicsPipeline(pipeline_info);

    /*
    color_blending.attachmentCount = 0;

    auto vertex_module = engine->renderer->getShader("shaders/ffxiactor_shadow_vert.spv");
    auto fragment_module = engine->renderer->getShader("shaders/shadow_frag.spv");

    vert_shader_stage_info.module = *vertex_module;
    frag_shader_stage_info.module = *fragment_module;

    shaderStages = {vert_shader_stage_info, frag_shader_stage_info};

    pipeline_info.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipeline_info.pStages = shaderStages.data();

    rasterizer.depthClampEnable = true;
    rasterizer.depthBiasEnable = true;

    dynamic_states.push_back(vk::DynamicState::eDepthBias);
    dynamic_state_ci.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_ci.pDynamicStates = dynamic_states.data();
    pipeline_shadowmap = engine->renderer->createShadowmapPipeline(pipeline_info);
    */
}
