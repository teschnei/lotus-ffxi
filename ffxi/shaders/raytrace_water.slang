import lotus;
import lotus.raytrace;
import pbr;

[shader("closesthit")]
void ClosestHit(inout lotus::RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    RayDesc newRay;
    newRay.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    newRay.TMin = 0.0;
    newRay.Direction = WorldRayDirection();
    newRay.TMax = 1000.0 - RayTCurrent();
    TraceRay(topLevelAS, 0, 0x01 | 0x02 | 0x10, 0, 0, 0, newRay, payload);
}
