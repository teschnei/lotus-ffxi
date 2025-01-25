import lotus;
import lotus.raytrace;
import pbr;

struct MMBVertex
{
    float3 pos;
    float3 norm;
    float4 colour;
    float2 uv;
}

[shader("closesthit")]
void ClosestHit(inout lotus::RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    payload.id = lotus::GetMeshID();
    float distance = payload.distance + RayTCurrent();
    payload.distance = distance;
    if (distance > lights[0].light.landscape.max_fog)
    {
        if (payload.depth == 0)
        {
            payload.bxdf = pbr::ConstantBxDF(lights[0].light.landscape.fog_color.rgb, 1).Serialize();
        }
        else
        {
            payload.bxdf = pbr::ConstantBxDF(0, 1).Serialize();
        }
        payload.distance = 1000.f;
        return;
    }

    lotus::Mesh mesh = lotus::GetMeshInfo();

    uint16_t3 indices = ((uint16_t3*)mesh.index_buffer)[PrimitiveIndex()];

    MMBVertex* vertices = (MMBVertex*)mesh.vertex_buffer;
    MMBVertex v0 = vertices[indices.x];
    MMBVertex v1 = vertices[indices.y];
    MMBVertex v2 = vertices[indices.z];

    const float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    float3 normal = v0.norm * barycentrics.x + v1.norm * barycentrics.y + v2.norm * barycentrics.z;
    float3 transformed_normal = mul(float3x3(ObjectToWorld3x4()), normal);
    float3 normalized_normal = normalize(transformed_normal);

    float3 primitive_colour = (v0.colour * barycentrics.x + v1.colour * barycentrics.y + v2.colour * barycentrics.z).xyz;

    float dot_product = dot(-lights[0].light.diffuse_dir, normalized_normal);

    float2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;
    uv += mesh.uv_offset;

    lotus::Material material = *(lotus::Material*)mesh.material;
    float3 texture_color = textures[NonUniformResourceIndex(material.texture_index)].SampleLevel(uv, 0).xyz;

    float3 transformed_v0 = mul(float3x3(ObjectToWorld3x4()), v0.pos);
    float3 transformed_v1 = mul(float3x3(ObjectToWorld3x4()), v1.pos);
    float3 transformed_v2 = mul(float3x3(ObjectToWorld3x4()), v2.pos);
    float3 vertex_vec1 = normalize(float3(transformed_v1 - transformed_v0));
    float3 vertex_vec2 = normalize(float3(transformed_v2 - transformed_v0));

    float3 cross_vec = normalize(cross(vertex_vec1, vertex_vec2));

    if ((dot(cross_vec, normalized_normal)) < 0)
        cross_vec = -cross_vec;


    float3 albedo = texture_color.rgb * primitive_colour;

    //if (distance > lights[0].light.landscape.min_fog && distance < lights[0].light.landscape.max_fog)
    {
        //albedo = mix(albedo, light.light.landscape.fog_color.rgb, (distance - light.light.landscape.min_fog) / (light.light.landscape.max_fog - light.light.landscape.min_fog));
        //diffuse = mix(diffuse, float3(M_PI), (distance - light.light.landscape.min_fog) / (light.light.landscape.max_fog - light.light.landscape.min_fog));
    }

    payload.face_normal = cross_vec;
    payload.intersection = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    payload.prev_pos = payload.intersection;
    payload.normal = normalized_normal;
    payload.albedo = albedo;
    payload.bxdf = pbr::DiffuseBxDF(albedo).Serialize();
}

[shader("anyhit")]
void AnyHit(inout lotus::RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    lotus::Mesh mesh = lotus::GetMeshInfo();

    uint16_t3 indices = ((uint16_t3*)mesh.index_buffer)[PrimitiveIndex()];

    MMBVertex* vertices = (MMBVertex*)mesh.vertex_buffer;
    MMBVertex v0 = vertices[indices.x];
    MMBVertex v1 = vertices[indices.y];
    MMBVertex v2 = vertices[indices.z];

    const float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    float2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;
    uv += mesh.uv_offset;

    lotus::Material material = *(lotus::Material*)mesh.material;
    float alpha = textures[NonUniformResourceIndex(material.texture_index)].SampleLevel(uv, 0).a;
    if (alpha < 1.f/32.f)
        IgnoreHit();
}
