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
            float u = (float)x / screen.GetHeight();
            float v = (float)y / screen.GetWidth();
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

Pixel Trace(const Ray &ray, const Scene& scene)
{
    uint3 c = uint3();
    //c.x = clamp(ray.dir.x,0.f,1.f) * 0xff;
    //c.y = clamp(ray.dir.y,0.f,1.f) * 0xff;
    //c.z = clamp(ray.dir.z,0.f,1.f) * 0xff;
    c.x = ray.dir.z * 0xff;

    c = clamp(c, 0, 0xff);
    // Convert to color
    const Pixel color = ((c.x & 0x0ff) << 16) | ((c.y & 0x0ff) << 8) | (c.z & 0x0ff);

    return color;
}