import lotus;

[vk_binding(0)] ConstantBuffer<lotus::Camera> camera;
[vk_binding(1)] ConstantBuffer<lotus::ModelUBO> model;

struct VSInput
{
    float3 position;
    float3 normal;
    float4 colour;
    float2 uv;
};

struct Params
{
    float4 colour : COLOUR;
    float2 uv : UV;
    float3 position : POSITION;
    float3 normal : NORMAL;
}

[shader("vertex")]
float4 Vertex(VSInput input, out Params output) : SV_Position
{
    float4 worldPos = mul(model.model, float4(input.position, 1.0));
    output.position = worldPos.xyz;
    output.colour = input.colour;
    output.uv = input.uv;
    output.normal = normalize(mul(float3x3(model.model_IT), input.normal));
    return mul(camera.proj, mul(camera.view, worldPos));
}

[vk_binding(0,1)] StructuredBuffer<lotus::Mesh> meshes;
[vk_binding(1,1)] Sampler2D textures[];

struct FSOutput
{
    float4 accumulation;
    float revealage;
    float4 particle;
}

[vk_push_constant]
uniform uint mesh_index;

[shader("fragment")]
FSOutput FragmentAdd(Params input) {
    FSOutput output;
    lotus::Mesh mesh = meshes[mesh_index];
    lotus::Material mat = *(lotus::Material*)mesh.material;

    float4 texture_colour = textures[mat.texture_index].Sample(input.uv);
    uint bc2_alpha = uint((texture_colour.a * 255.f)) >> 4;
    if (bc2_alpha == 0)
        discard;
    float texture_alpha = float(bc2_alpha) / 8.0;
    float4 mesh_colour = mesh.colour;
    float3 colour = input.colour.rgb * mesh_colour.rgb * texture_colour.rgb;
    float alpha = input.colour.a * mesh_colour.a * texture_alpha;
    output.particle = float4(colour * alpha, 1.0);
    return output;
}

[shader("fragment")]
FSOutput FragmentBlend(Params input, float4 fragCoord : SV_Position) {
    FSOutput output;
    lotus::Mesh mesh = meshes[mesh_index];
    lotus::Material mat = *(lotus::Material*)mesh.material;

    float4 texture_colour = textures[mat.texture_index].Sample(input.uv);
    float4 mesh_colour = mesh.colour;
    float3 colour = input.colour.rgb *  mesh_colour.rgb * texture_colour.rgb * 4;
    float alpha = input.colour.a * mesh_colour.a * texture_colour.a * 2;

    float a = min(1.0, alpha) * 8.0 + 0.01;
    float b = -(1.0 - fragCoord.z) * 0.95 + 1.0;
    float w = clamp(a * a * a * 1e8 * b * b * b, 1e-2, 3e2);
    output.accumulation.rgb = colour * w;
    output.accumulation.a = alpha * w;
    output.revealage = alpha;
    return output;
}

