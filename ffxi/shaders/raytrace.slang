import lotus;
import lotus.raytrace;
import pbr;

[shader("miss")]
void Miss(inout lotus::RayPayload payload)
{
    payload.normal = float3(1.0);
    payload.distance = payload.distance + RayTCurrent();
    payload.prev_pos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    payload.intersection = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float dot_up = dot(WorldRayDirection(), float3(0.f, -1.f, 0.f));
    float3 diffuse_difference = lights[0].light_info.pos - WorldRayOrigin();
    float3 diffuse_direction = normalize(diffuse_difference);
    float dot_diffuse = abs(dot(WorldRayDirection(), diffuse_direction));
    float3 perp = normalize(diffuse_direction.z < diffuse_direction.x ? float3(diffuse_direction.y,-diffuse_direction.x,0) : float3(0,-diffuse_direction.z,diffuse_direction.y));
    float3 maximal_direction = normalize(lights[0].light_info.pos + (perp * lights[0].light_info.radius) - WorldRayOrigin());
    float maximal_dot = abs(dot(diffuse_direction, maximal_direction));
    if (dot_diffuse > maximal_dot)
    {
        //payload.bxdf = pbr::ConstantBxDF(lights[0].light_info.colour, 100).Serialize();
        //return;
    }

    if (dot_up < lights[0].light.skybox_altitudes2)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes1) / (lights[0].light.skybox_altitudes2 - lights[0].light.skybox_altitudes1);
        payload.bxdf = pbr::ConstantBxDF(lerp(lights[0].light.skybox_colors[0], lights[0].light.skybox_colors[1], value).xyz, 1).Serialize();
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes3)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes2) / (lights[0].light.skybox_altitudes3 - lights[0].light.skybox_altitudes2);
        payload.bxdf = pbr::ConstantBxDF(lerp(lights[0].light.skybox_colors[1], lights[0].light.skybox_colors[2], value).xyz, 1).Serialize();
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes4)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes3) / (lights[0].light.skybox_altitudes4 - lights[0].light.skybox_altitudes3);
        payload.bxdf = pbr::ConstantBxDF(lerp(lights[0].light.skybox_colors[2], lights[0].light.skybox_colors[3], value).xyz, 1).Serialize();
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes5)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes4) / (lights[0].light.skybox_altitudes5 - lights[0].light.skybox_altitudes4);
        payload.bxdf = pbr::ConstantBxDF(lerp(lights[0].light.skybox_colors[3], lights[0].light.skybox_colors[4], value).xyz, 1).Serialize();
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes6)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes5) / (lights[0].light.skybox_altitudes6 - lights[0].light.skybox_altitudes5);
        payload.bxdf = pbr::ConstantBxDF(lerp(lights[0].light.skybox_colors[4], lights[0].light.skybox_colors[5], value).xyz, 1).Serialize();
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes7)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes6) / (lights[0].light.skybox_altitudes7 - lights[0].light.skybox_altitudes6);
        payload.bxdf = pbr::ConstantBxDF(lerp(lights[0].light.skybox_colors[5], lights[0].light.skybox_colors[6], value).xyz, 1).Serialize();
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes8)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes7) / (lights[0].light.skybox_altitudes8 - lights[0].light.skybox_altitudes7);
        payload.bxdf = pbr::ConstantBxDF(lerp(lights[0].light.skybox_colors[6], lights[0].light.skybox_colors[7], value).xyz, 1).Serialize();
        return;
    }
}
