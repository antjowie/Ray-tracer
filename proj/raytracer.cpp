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
    for (unsigned y = 0; y < screen.GetHeight(); y++)
    {
        for (unsigned x = 0; x < screen.GetWidth(); x++)
        {
            float u = x / screen.GetHeight();
            float v = y / screen.GetWidth();
            float3 P = p0 + u * right + v * down;
            float3 D = normalize(P - E);

            Ray ray;
            ray.origin = E;
            ray.dir = D;
            float3 c = Trace(ray,scene);
            
            screen.Plot(x, y, make_uint3(c));
        }
    }
}

float3 Trace(Ray ray, const Scene& scene)
{
    return ray.dir * 256.f;
}