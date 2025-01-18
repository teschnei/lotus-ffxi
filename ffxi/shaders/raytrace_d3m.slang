import lotus;
import lotus.raytrace;
import pbr;

struct D3MVertex
{
    float3 pos;
    float3 norm;
    float4 colour;
    float2 uv;
}

void ClosestHitCommon(inout lotus::RayPayload payload, float2 barycentrics)
{
    if (RayTCurrent() > lights[0].light.entity.max_fog)
    {
        payload.diffuse = float3(M_PI);
        payload.bsdf = pbr::BSDFSample(lights[0].light.entity.fog_color.rgb / M_PI, 1, float3(0));
        payload.depth = 10;
        payload.distance = RayTCurrent();
        return;
    }
    payload.depth++;

    lotus::Mesh mesh = lotus::GetMeshInfo();

    uint16_t3 indices = ((uint16_t3*)mesh.index_buffer)[PrimitiveIndex()];

    D3MVertex* vertices = (D3MVertex*)mesh.vertex_buffer;
    D3MVertex v0 = vertices[indices.x];
    D3MVertex v1 = vertices[indices.y];
    D3MVertex v2 = vertices[indices.z];

    const float3 barycentrics = float3(1.0 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);

    float3 normal = v0.norm * barycentrics.x + v1.norm * barycentrics.y + v2.norm * barycentrics.z;
    float3 transformed_normal = mul(float3x3(ObjectToWorld3x4()), normal);
    float3 normalized_normal = normalize(transformed_normal);

    float4 primitive_colour = (v0.colour * barycentrics.x + v1.colour * barycentrics.y + v2.colour * barycentrics.z);

    float dot_product = dot(-lights[0].light.diffuse_dir, normalized_normal);

    float2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;
    uv += mesh.uv_offset;

    lotus::Material material = *(lotus::Material*)mesh.material;
    float4 texture_colour = textures[NonUniformResourceIndex(material.texture_index)].SampleLevel(uv, 0);

    float3 colour = primitive_colour.rgb * mesh.colour.rgb * texture_colour.rgb * 4;
    float a = primitive_colour.a * mesh.colour.a * texture_colour.a * 2;
    colour *= a;

    float3 transformed_v0 = mul(float3x3(ObjectToWorld3x4()), v0.pos);
    float3 transformed_v1 = mul(float3x3(ObjectToWorld3x4()), v1.pos);
    float3 transformed_v2 = mul(float3x3(ObjectToWorld3x4()), v2.pos);
    float3 vertex_vec1 = normalize(float3(transformed_v1 - transformed_v0));
    float3 vertex_vec2 = normalize(float3(transformed_v2 - transformed_v0));

    float3 cross_vec = normalize(cross(vertex_vec1, vertex_vec2));

    if ((dot(cross_vec, normalized_normal)) < 0)
        cross_vec = -cross_vec;

    float3 origin = WorldRayOrigin() + (WorldRayDirection() * RayTCurrent()) + cross_vec * 0.001;
    uint flags = 0x01 | 0x02 | 0x20;
    if (payload.depth < 3)
        flags |= 0x10;
    RayDesc ray;
    ray.Origin = origin.xyz;
    ray.TMin = 0.0;
    ray.Direction = WorldRayDirection() - RayTCurrent();
    ray.TMax = 1000.0;
    TraceRay(topLevelAS, 0, flags, 0, 0, 0, ray, payload);
    payload.particle += colour;
}

void AnyHitCommon(inout lotus::RayPayload payload, float2 barycentrics)
{
    lotus::Mesh mesh = lotus::GetMeshInfo();

    uint16_t3 indices = ((uint16_t3*)mesh.index_buffer)[PrimitiveIndex()];

    D3MVertex* vertices = (D3MVertex*)mesh.vertex_buffer;
    D3MVertex v0 = vertices[indices.x];
    D3MVertex v1 = vertices[indices.y];
    D3MVertex v2 = vertices[indices.z];

    const float3 barycentrics = float3(1.0 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);

    float2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;
    float4 primitive_colour = (v0.colour * barycentrics.x + v1.colour * barycentrics.y + v2.colour * barycentrics.z);
    uv += mesh.uv_offset;

    lotus::Material material = *(lotus::Material*)mesh.material;
    float4 texture_colour = textures[NonUniformResourceIndex(material.texture_index)].SampleLevel(uv, 0);

    float3 colour = primitive_colour.rgb * mesh.colour.rgb * texture_colour.rgb * 4;
    float a = primitive_colour.a * mesh.colour.a * texture_colour.a * 2;
    colour *= a;

    if (colour.r <= 0.f && colour.g <= 0.f && colour.b <= 0.f)
        IgnoreHit();
}

[shader("closesthit")]
void ClosestHit(inout lotus::RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    return ClosestHitCommon(payload, attribs.barycentrics);
}

[shader("anyhit")]
void AnyHit(inout lotus::RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    return AnyHitCommon(payload, attribs.barycentrics);
}

[shader("anyhit")]
void AnyHitShadow(lotus::RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    IgnoreHit();
}

struct HitAttributes
{
    float2 barycentrics;
    uint primitive_id;
}

[shader("intersection")]
void Intersection()
{
    lotus::Mesh mesh = lotus::GetMeshInfo();
    D3MVertex* vertices = (D3MVertex*)mesh.vertex_buffer;

    for (uint i = 0; i < mesh.index_count / 3; ++i)
    {
        uint16_t3 indices = ((uint16_t3*)mesh.index_buffer)[i];

        float3x4 transform = ObjectToWorld3x4();
        /*
        if (mesh_info.billboard > 0)
        {
            float4x4 scale = float4x4(float4(mesh_info.scale.x, 0, 0, 0), float4(0, mesh_info.scale.y, 0, 0), float4(0, 0, mesh_info.scale.z, 0), float4(0,0,0,1));
            float4x4 pos = float4x4(1.0);
            pos[3] = float4(transform[3], 1.0);
            float4x4 rot = float4x4(transpose(mat3(lookAt(gl_WorldRayOriginEXT, normalize(gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT), float3(0, -1.0, 0)))));
            transform = float4x3(pos * rot * scale);
        }
        */

        float3 v0 = mul(transform, float4(vertices[indices.x].pos, 1.0));
        float3 v1 = mul(transform, float4(vertices[indices.y].pos, 1.0));
        float3 v2 = mul(transform, float4(vertices[indices.z].pos, 1.0));

        float3 e1 = v1 - v0;
        float3 e2 = v2 - v0;

        float3 h = cross(WorldRayDirection(), e2);
        float a = dot(e1, h);

        if (a > -0.00001 && a < 0.00001)
            continue;

        float f = 1/a;
        float3 s = WorldRayOrigin() - v0;
        float u = f * dot(s, h);

        if (u < 0.0 || u > 1.0)
            continue;

        float3 q = cross(s, e1);
        float v = f * dot(WorldRayDirection(), q);

        if (v < 0.0 || u + v > 1.0)
            continue;

        float t = f * dot(e2, q);

        if (t > RayTMin() && t < RayTCurrent())
        {
            HitAttributes attribs;
            attribs.barycentrics = float2(u, v);
            attribs.primitive_id = i;
            if (ReportHit(t, 0u, attribs))
                return;
        }
    }
}

[shader("closesthit")]
void ClosestHitAABB(inout lotus::RayPayload payload, HitAttributes attribs)
{
    return ClosestHitCommon(payload, attribs.barycentrics);
}

[shader("anyhit")]
void AnyHitAABB(inout lotus::RayPayload payload, HitAttributes attribs)
{
    return AnyHitCommon(payload, attribs.barycentrics);
}

[shader("anyhit")]
void AnyHitShadowAABB(lotus::RayPayload payload, HitAttributes attribs)
{
    IgnoreHit();
}
