import lotus;
import lotus.raytrace;
import pbr;

[shader("miss")]
void Miss(inout lotus::RayPayload payload)
{
    payload.diffuse = float3(M_PI);
    payload.depth = 10;
    payload.normal = float3(1.0);
    payload.distance = payload.distance + RayTCurrent();
    payload.prev_pos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    payload.origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float dot_up = dot(WorldRayDirection(), float3(0.f, -1.f, 0.f));

    if (dot_up < lights[0].light.skybox_altitudes2)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes1) / (lights[0].light.skybox_altitudes2 - lights[0].light.skybox_altitudes1);
        payload.BRDF = lerp(lights[0].light.skybox_colors[0], lights[0].light.skybox_colors[1], value).xyz / M_PI;
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes3)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes2) / (lights[0].light.skybox_altitudes3 - lights[0].light.skybox_altitudes2);
        payload.BRDF = lerp(lights[0].light.skybox_colors[1], lights[0].light.skybox_colors[2], value).xyz / M_PI;
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes4)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes3) / (lights[0].light.skybox_altitudes4 - lights[0].light.skybox_altitudes3);
        payload.BRDF = lerp(lights[0].light.skybox_colors[2], lights[0].light.skybox_colors[3], value).xyz / M_PI;
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes5)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes4) / (lights[0].light.skybox_altitudes5 - lights[0].light.skybox_altitudes4);
        payload.BRDF = lerp(lights[0].light.skybox_colors[3], lights[0].light.skybox_colors[4], value).xyz / M_PI;
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes6)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes5) / (lights[0].light.skybox_altitudes6 - lights[0].light.skybox_altitudes5);
        payload.BRDF = lerp(lights[0].light.skybox_colors[4], lights[0].light.skybox_colors[5], value).xyz / M_PI;
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes7)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes6) / (lights[0].light.skybox_altitudes7 - lights[0].light.skybox_altitudes6);
        payload.BRDF = lerp(lights[0].light.skybox_colors[5], lights[0].light.skybox_colors[6], value).xyz / M_PI;
        return;
    }
    if (dot_up < lights[0].light.skybox_altitudes8)
    {
        float value = (max(dot_up, 0.0) - lights[0].light.skybox_altitudes7) / (lights[0].light.skybox_altitudes8 - lights[0].light.skybox_altitudes7);
        payload.BRDF = lerp(lights[0].light.skybox_colors[6], lights[0].light.skybox_colors[7], value).xyz / M_PI;
        return;
    }
}
