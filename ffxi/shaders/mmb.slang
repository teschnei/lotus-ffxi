import lotus;

[vk_binding(0)] ConstantBuffer<lotus::Camera> camera[2];

struct VSInput
{
    float3 position;
    float3 normal;
    float3 colour;
    float2 texCoord;
    float4x4 instanceMat;
    float4x4 instanceMat_T;
    float3x3 instanceMat_IT;
}

struct Params
{
    float4 fragColour : COLOUR;
    float2 fragTexCoord : FRAGCOORD;
    float3 fragPos : FRAGPOS;
    float3 normal : NORMAL;
    float4 pos : POSITION;
    float4 prevPos : PREVPOSITION;
}

[shader("vertex")]
float4 Vertex(VSInput input, out Params output) : SV_Position
{
    output.pos = mul(camera[0].proj, mul(camera[0].view, mul(input.instanceMat, float4(input.position, 1.0))));
    output.prevPos = mul(camera[1].proj, mul(camera[1].view, mul(input.instanceMat, float4(input.position, 1.0))));
    output.fragColour = float4(input.colour, 1.0);
    output.fragTexCoord = input.texCoord;

    output.fragPos = (mul(input.instanceMat, float4(input.position, 1.0))).xyz;
    output.normal = normalize(mul(input.instanceMat_IT, input.normal));
    return output.pos;
}

[vk_binding(0,1)] StructuredBuffer<lotus::Mesh> meshes;
[vk_binding(1,1)] Sampler2D textures[];

struct FSOutput
{
    float4 position;
    float4 normal;
    float4 faceNormal;
    float4 albedo;
    uint materialIndex;
    uint lightType;
    float4 motionVector;
}

[vk_push_constant]
uniform uint mesh_index;

[shader("fragment")]
FSOutput FragmentOpaque(Params input) {
    FSOutput output;
    output.position = float4(input.fragPos, 1.0);
    output.normal = float4(input.normal, 1.0);
    float3 dx = ddx(input.fragPos);
    float3 dy = ddy(input.fragPos);
    float3 cross_vec = normalize(cross(dx, dy));
    if ((dot(cross_vec, input.normal)) < 0)
        cross_vec = -cross_vec;

    lotus::Mesh mesh = meshes[mesh_index];
    lotus::Material mat = *(lotus::Material*)mesh.material;

    output.faceNormal = float4(cross_vec, 1.0);
    output.albedo = textures[mat.texture_index].Sample(input.fragTexCoord);
    output.albedo.rgb *= input.fragColour.rgb;

    /*
    if (outAlbedo.a > 1.f/32.f)
        outAlbedo.a = 1;
    else
        outAlbedo.a = 0;
    */
    output.albedo.a = 1;

    output.materialIndex = mesh_index;
    output.lightType = mat.light_type;
    float2 curScreenPos = (input.pos.xy / input.pos.w) * 0.5 + 0.5;
    float2 prevScreenPos = (input.prevPos.xy / input.prevPos.w) * 0.5 + 0.5;
    output.motionVector.xy = float2(curScreenPos - prevScreenPos);
    output.motionVector.zw = float2(input.pos.z, input.prevPos.z);
    return output;
}

[shader("fragment")]
FSOutput FragmentBlend(Params input) {
    FSOutput output;
    output.position = float4(input.fragPos, 1.0);
    output.normal = float4(input.normal, 1.0);
    float3 dx = ddx(input.fragPos);
    float3 dy = ddy(input.fragPos);
    float3 cross_vec = normalize(cross(dx, dy));
    if ((dot(cross_vec, input.normal)) < 0)
        cross_vec = -cross_vec;

    lotus::Mesh mesh = meshes[mesh_index];
    lotus::Material mat = *(lotus::Material*)mesh.material;

    output.faceNormal = float4(cross_vec, 1.0);
    output.albedo = textures[mat.texture_index].Sample(input.fragTexCoord);
    uint bc2_alpha = uint((output.albedo.a * 255.f)) >> 4;
    if (bc2_alpha == 0)
        discard;
    output.albedo.a = float(bc2_alpha) / 8.0;
    output.albedo *= input.fragColour;
    output.materialIndex = mesh_index;
    output.lightType = mat.light_type;
    float2 curScreenPos = (input.pos.xy / input.pos.w) * 0.5 + 0.5;
    float2 prevScreenPos = (input.prevPos.xy / input.prevPos.w) * 0.5 + 0.5;
    output.motionVector.xy = float2(curScreenPos - prevScreenPos);
    output.motionVector.zw = float2(input.pos.z, input.prevPos.z);
    return output;
}
