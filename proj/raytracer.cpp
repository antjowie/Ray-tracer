#include "precomp.h"


PrimaryHit TriangleIntersect(const Ray& ray, const float3* v3)
{
    PrimaryHit hit;

    //https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    const float EPSILON = 0.0000001;
    float3 vertex0 = v3[0];
    float3 vertex1 = v3[1];
    float3 vertex2 = v3[2];
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
    u = f * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return hit;
    q = cross(s, edge1);
    v = f * dot(ray.dir, q);
    if (v < 0.0 || u + v > 1.0)
        return hit;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot(edge2, q);
    if (t > EPSILON) // ray intersection
    {
        hit.isHit = true;
        hit.t = t;
        hit.hit = ray.origin + ray.dir * t;
        //TODO hit.surfaceNormal = 

        return hit;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return hit;
}

PrimaryHit Trace(const Ray& ray, const Scene& scene)
{
    float d = -1.f;
    PrimaryHit ret;

    for (const auto& model : scene.GetModels())
    {
        for (const auto& mesh : model.meshes)
        {
            const auto& ver = mesh.vertices;
            const auto& ind = mesh.indices;

            // Iterate over 9 indices per loop
            for (size_t i = 0; i < ind.size(); i += 3)
            {
                // Create vertex from 3 indicess
                float3 v[3];
                for (size_t j = 0; j < 3; j++)
                {
                    v[j] = make_float3(
                        ver[ind[i + j] * 3 + 0], 
                        ver[ind[i + j] * 3 + 1], 
                        ver[ind[i + j] * 3 + 2]);
                }

                auto h = TriangleIntersect(ray, v);
                if (h.isHit)
                {
                    // If no hit yet
                    if (!ret.isHit || h.t < ret.t)
                    {
                        ret = h;
                        ret.model = &model;
                        ret.mesh = &mesh;
                    }
                }
            }
        }
    }
    return ret;
}

void RenderArea(Pixel* buffer, uint x, uint y, uint w, uint h, uint bw, uint bh)
{

}

void Renderer::Render(const mat4& t, Surface& screen, const Scene& scene)
{
    // Calculate eye and screen
    float3 p0 = t.TransformPoint(make_float3(-1, 1, 1)); // top-left
    float3 p1 = t.TransformPoint(make_float3(1, 1, 1)); // top-right
    float3 p2 = t.TransformPoint(make_float3(-1, -1, 1)); // bottom-left
    float3 E = t.TransformPoint(make_float3(0, 0, 0));
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
            
            auto hit = Trace(ray, scene);
            Pixel c;
            
            if (hit.isHit)
            {
                c = hit.mesh->mat.color;
            }
            else
            {
                float3 normalizedColor = ray.dir * 0xff;
                c = ((static_cast<uint>(std::fabsf(normalizedColor.x)) & 0xFF)) |
                    ((static_cast<uint>(std::fabsf(normalizedColor.y)) & 0xFF) << 8) |
                    ((static_cast<uint>(std::fabsf(normalizedColor.z)) & 0xFF) << 16);
            }

            screen.Plot(x, y, c);
        }
    }
}