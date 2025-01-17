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
    float distance = payload.distance + RayTCurrent();
    payload.distance = distance;
    if (distance > lights[0].light.landscape.max_fog)
    {
        payload.diffuse = float3(M_PI);
        payload.BRDF = lights[0].light.landscape.fog_color.rgb / M_PI;
        payload.depth = 10;
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

    float3 trace_origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent() + cross_vec * 0.001;
    float3 diffuse = float3(0);//light.light.landscape.ambient_color.rgb * light.light.landscape.brightness;

    for (int i = 0; i < lights[0].light.light_count; i++)
    {
        float2 u = float2(lotus.rand::rnd(payload.seed), lotus.rand::rnd(payload.seed));
        diffuse += lotus::TraceLight(lights[0].light_info, trace_origin, normalized_normal, u);
    }

    float3 tangent, bitangent;
    float pdf;
    pbr::createCoordinateSystem(normalized_normal, tangent, bitangent);
    float3 sample = pbr::SampleCosineHemisphere(float2(lotus.rand::rnd(payload.seed), lotus.rand::rnd(payload.seed)));
    payload.direction = normalize(sample.x * tangent + sample.y * bitangent + sample.z * normalized_normal);
    payload.origin = trace_origin.xyz;
    payload.prev_pos = trace_origin.xyz;

    //const float p = cos_theta / M_PI;

    float cos_theta = dot(payload.direction, normalized_normal);
    float3 albedo = texture_color.rgb * primitive_colour;

    //if (distance > lights[0].light.landscape.min_fog && distance < lights[0].light.landscape.max_fog)
    {
        //albedo = mix(albedo, light.light.landscape.fog_color.rgb, (distance - light.light.landscape.min_fog) / (light.light.landscape.max_fog - light.light.landscape.min_fog));
        //diffuse = mix(diffuse, float3(M_PI), (distance - light.light.landscape.min_fog) / (light.light.landscape.max_fog - light.light.landscape.min_fog));
    }

    float3 BRDF = albedo / M_PI;
    payload.BRDF = BRDF;
    payload.diffuse = diffuse;
    payload.weight = M_PI;
    payload.normal = normalized_normal;

    /*
    if (material.ior > 0)
    {
        //TESTING
        //transform wo relative to surface normal
        float3 wo = float3(dot(-gl_WorldRayDirectionEXT, tangent), dot(-gl_WorldRayDirectionEXT, bitangent), dot(-gl_WorldRayDirectionEXT, normalized_normal));

        BSDFSample brdf = sampleBRDF(payload.seed, wo, float3(material.ior + 1), -texture_color.rgb, sqrt(material.roughness));
        float3 wi_world = float3(brdf.wi.x * tangent + brdf.wi.y * bitangent + brdf.wi.z * normalized_normal);
        payload.BRDF = brdf.f;
        payload.weight = brdf.pdf_inv * abs(dot(wi_world, normalized_normal));
        payload.diffuse = float3(0);
        payload.direction = wi_world;
        if (brdf.pdf_inv == 0)
            payload.depth = 10;
    }
    */
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
