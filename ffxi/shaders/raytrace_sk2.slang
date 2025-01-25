import lotus;
import lotus.raytrace;
import pbr;

struct SK2Vertex
{
    float3 pos;
    float3 norm;
    float2 uv;
}

[shader("closesthit")]
void ClosestHit(inout lotus::RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    payload.id = lotus::GetMeshID();
    float distance = payload.distance + RayTCurrent();
    payload.distance = distance;
    if (distance > lights[0].light.entity.max_fog)
    {
        if (payload.depth == 0)
        {
            payload.bxdf = pbr::ConstantBxDF(lights[0].light.entity.fog_color.rgb, 1).Serialize();
        }
        payload.depth = 10;
        return;
    }

    lotus::Mesh mesh = lotus::GetMeshInfo();

    uint16_t3 indices = ((uint16_t3*)mesh.index_buffer)[PrimitiveIndex()];

    SK2Vertex* vertices = (SK2Vertex*)mesh.vertex_buffer;
    SK2Vertex v0 = vertices[indices.x];
    SK2Vertex v1 = vertices[indices.y];
    SK2Vertex v2 = vertices[indices.z];

    const float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    float3 normal = v0.norm * barycentrics.x + v1.norm * barycentrics.y + v2.norm * barycentrics.z;
    float3 transformed_normal = mul(float3x3(ObjectToWorld3x4()), normal);
    float3 normalized_normal = normalize(transformed_normal);
    // SK2 are double sided
    if (dot(-WorldRayDirection(), normalized_normal) < 0)
    {
        normalized_normal = -normalized_normal;
    }

    float dot_product = dot(-lights[0].light.diffuse_dir, normalized_normal);

    float2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;

    lotus::Material material = *(lotus::Material*)mesh.material;
    float4 texture_color = textures[NonUniformResourceIndex(material.texture_index)].SampleLevel(uv, 0);

    float3 transformed_v0 = mul(float3x3(ObjectToWorld3x4()), v0.pos);
    float3 transformed_v1 = mul(float3x3(ObjectToWorld3x4()), v1.pos);
    float3 transformed_v2 = mul(float3x3(ObjectToWorld3x4()), v2.pos);
    float3 vertex_vec1 = normalize(float3(transformed_v1 - transformed_v0));
    float3 vertex_vec2 = normalize(float3(transformed_v2 - transformed_v0));

    float3 cross_vec = normalize(cross(vertex_vec1, vertex_vec2));

    if ((dot(cross_vec, normalized_normal)) < 0)
        cross_vec = -cross_vec;

    float3 albedo = texture_color.rgb;

    payload.normal = normalized_normal;
    payload.face_normal = cross_vec;
    payload.intersection = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    payload.albedo = albedo;

    if (payload.depth == 0)
    {
        SK2Vertex* prev_vertices = (SK2Vertex*)mesh.vertex_prev_buffer;
        float3 prev_vertex_pos = prev_vertices[indices.x].pos * barycentrics.x +
                                 prev_vertices[indices.y].pos * barycentrics.y +
                                 prev_vertices[indices.z].pos * barycentrics.z;
        payload.prev_pos = (mul(mesh.model_prev, float4(prev_vertex_pos, 1.0))).xyz;
    }

    //if (distance > lights[0].light.entity.min_fog && distance < lights[0].light.entity.max_fog)
    {
        //albedo = mix(albedo, light.light.entity.fog_color.rgb, (distance - light.light.entity.min_fog) / (light.light.entity.max_fog - light.light.entity.min_fog));
        //diffuse = mix(diffuse, float3(M_PI), (distance - light.light.entity.min_fog) / (light.light.entity.max_fog - light.light.entity.min_fog));
    }

    if (material.ior > 0 && texture_color.a > 0)
    {
        //apply the texture's "shininess" (inverse roughness)
        float2 base_shininess = float2(1); // TODO: material.roughness
        float2 roughness = 1.0 - ((base_shininess) * texture_color.a);
        var diffuse_bxdf = pbr::DiffuseBxDF(albedo);
        var dielectric_bxdf = pbr::DielectricBxDF(1.5, sqrt(roughness));
        payload.bxdf = pbr::DielectricDiffuseBxDF(dielectric_bxdf, diffuse_bxdf).Serialize();
    }
    else
    {
        payload.bxdf = pbr::DiffuseBxDF(albedo).Serialize();
    }
}

[shader("anyhit")]
void AnyHit(inout lotus::RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    lotus::Mesh mesh = lotus::GetMeshInfo();

    uint16_t3 indices = ((uint16_t3*)mesh.index_buffer)[PrimitiveIndex()];

    SK2Vertex* vertices = (SK2Vertex*)mesh.vertex_buffer;
    SK2Vertex v0 = vertices[indices.x];
    SK2Vertex v1 = vertices[indices.y];
    SK2Vertex v2 = vertices[indices.z];

    const float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    float2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;

    lotus::Material material = *(lotus::Material*)mesh.material;
    float alpha = textures[NonUniformResourceIndex(material.texture_index)].SampleLevel(uv, 0).a;
    if (alpha < 1.f/32.f)
        IgnoreHit();
}
