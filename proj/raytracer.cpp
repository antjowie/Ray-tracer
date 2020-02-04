#include "precomp.h"

PrimaryHit TriangleIntersect(const Ray& ray, const float3& vertex0, const float3& vertex1, const float3& vertex2)
{
    PrimaryHit hit;

    //https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    const float EPSILON = 0.0000001;
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

// I think the template optimizes the bool call since it generates a function definition
template <bool earlyQuit = false>
PrimaryHit Trace(Ray ray, const Scene& scene)
{
    float d = -1.f;
    PrimaryHit ret;
    ret.t = std::numeric_limits<float>::max();

    // Get closest intersection
    for (const auto& model : scene.GetModels())
    {
        const BVHAccelerator::Hit& tris = model.bvh.Traverse(ray);

        auto tri = tris.triangles;
        for (int i = 0; i < tris.count; i++, tri++)
        {
            const auto& face = tri->face;

            auto h = TriangleIntersect(
                ray, face[0], face[1], face[2]);

            if (h.isHit)
            {
                // If no hit yet
                if (!ret.isHit || h.t < ret.t)
                {
                    ret = h;
                    ret.model = &model;
                    ret.mesh = tri->mesh;

                    ret.surfaceNormal = tri->normal;
                }

                if (earlyQuit)
                {
                    return ret;
                }
            }
        }        
    }

    // No hit
    if (!ret.isHit)
    {
        ret.color = ToPixel(ray.dir * 0xff);
        return ret;
    }

    // Cast shadow
    Pixel finalColor = 0;
    for(const auto& light: scene.GetLights())
    {
        float3 dir = normalize(light.pos - ret.hit);
        Ray shadow{ ret.hit + dir*0.0001f, dir };

        // Pronounced as s-hit
        auto shit = Trace<true>(shadow, scene);
        if(!shit.isHit)
        {
            // This is very incorrect but temp
            float l = 1.f; // Light intensity

            finalColor += ret.mesh->mat.color * l * max(0.f,dot(ret.surfaceNormal,shadow.dir));
        }
    }

    ret.color = finalColor;
    
    
    return ret;
}

/**
 * buffer: screen buffer
 * e: eye pos
 * x: initial x index
 * y: initial y index
 * w: width of area
 * h: height of area
 * bw: buffer width
 * hw: buffer height
 */
void RenderArea(
    Pixel* buffer, float3 e, float3 topLeft, float3 right, float3 down,
    uint x, uint y, uint w, uint h, uint bw, uint bh, const Scene& scene)
{
    // Iterate over area
    for (uint j = y; j < y + h; j++)
    {
        float v = (float)j / bh;
        for (uint i = x; i < x + w; i++)
        {
            // Generate ray
            float u = (float)i / bw;
            float3 P = topLeft + u * right + v * down;
            float3 D = normalize(P - e);

            Ray ray;
            ray.origin = e;
            ray.dir = D;

            auto hit = Trace(ray, scene);
            buffer[j * bw + i] = hit.color;
            //buffer[i * bw + j] = ScaleColor(c,dot(ray.dir,hit.surfaceNormal) * 0xff);
        }
    }
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

    const uint width = screen.GetWidth();
    const uint height = screen.GetHeight();

    // If we want no MT :(
    //RenderArea(screen.GetBuffer(), E, p0, right, down, 0, 0, width,height, width, height, scene);
    //return;

    // Calculate the tasks to render
    for (uint j = 0; j < height; j += squareY)
    {
        for (uint i = 0; i < width; i += squareX)
        {
            AddTask([=, &screen, &scene]()
            {
                RenderArea(
                    screen.GetBuffer(), E, p0, right, down, i, j, squareX, squareY, width, height, scene);
            });
        }
    }

    RunTasks();
    WaitForAll();

    taskflow.clear();
}