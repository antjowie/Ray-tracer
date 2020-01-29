#include "precomp.h"

void Render(const mat4& t, Surface& screen, const Scene& scene)
{
    // Calculate eye and screen
    float3 p0 = t * make_float3(-1, 1, 1); // top-left
    float3 p1 = t * make_float3(1, 1, 1); // top-right
    float3 p2 = t * make_float3(-1, -1, 1); // bottom-left
    float3 E = t * make_float3(0, 0, 0);
    float3 right = p1 - p0;
    float3 down = p2 - p0;

    // Calculate ray directions
    for (int y = 0; y < screen.GetHeight(); y++)
    {
        for (int x = 0; x < screen.GetWidth(); x++)
        {
            float u = (float)x / screen.GetWidth();
            float v = (float)y / screen.GetHeight();
            float3 P = p0 + u * right + v * down;
            float3 D = normalize(P - E);

            Ray ray;
            ray.origin = E;
            ray.dir = D;
            Pixel c = Trace(ray,scene);
            screen.Plot(x, y, c);
        }
    }
}

PrimaryHit TriangleIntersect(const Ray& ray, const Primitive& prim);

Pixel Trace(const Ray& ray, const Scene& scene)
{
    // Iterate over all primitives
    float d = -1.f;
    PrimaryHit hit;
    for (const auto& prim : scene.GetPrimitives())
    {
        auto h = TriangleIntersect(ray, prim);
        if (h.isHit)
        {
            // If no hit yet
            if (!hit.isHit) hit = h;
            else if (h.t < hit.t) hit = h;
        }
    }

    // TODO background color
    if (!hit.isHit) return 0;

    return hit.prim->color;

    //uint3 c = make_uint3(hit.prim.color);
    //c = clamp(c, 0, 0xff);
    //// Convert to color
    //const Pixel color = ((c.x & 0x0ff) << 16) | ((c.y & 0x0ff) << 8) | (c.z & 0x0ff);
    //
    //return color;
}

PrimaryHit TriangleIntersect(const Ray& ray, const Primitive& prim)
{
    PrimaryHit hit;

    //https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    const float EPSILON = 0.0000001;
    float3 vertex0 = prim.v[0];
    float3 vertex1 = prim.v[1];
    float3 vertex2 = prim.v[2];
    float3 edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = cross(ray.dir, edge2);
    a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return hit;    // This ray is parallel to this triangle.
    f = 1.0 / a;
    s = ray.origin - vertex0;
    u = f * dot(s,h);
    if (u < 0.0 || u > 1.0)
        return hit;
    q = cross(s,edge1);
    v = f * dot(ray.dir,q);
    if (v < 0.0 || u + v > 1.0)
        return hit;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot(edge2,q);
    if (t > EPSILON) // ray intersection
    {
        hit.isHit = true;
        hit.t = t;
        hit.hit = ray.origin + ray.dir * t;
        hit.prim = &prim;
        //TODO hit.surfaceNormal = 

        return hit;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return hit;
}