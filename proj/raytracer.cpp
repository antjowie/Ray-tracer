#include "precomp.h"

float TriangleIntersect(const Ray& ray, const float3& vertex0, const float3& vertex1, const float3& vertex2)
{
    //https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    const float EPSILON = 0.0000001;
    float3 edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = cross(ray.dir, edge2);
    a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return -1.f;    // This ray is parallel to this triangle.
    f = 1.0 / a;
    s = ray.origin - vertex0;
    u = f * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return -1.f;
    q = cross(s, edge1);
    v = f * dot(ray.dir, q);
    if (v < 0.0 || u + v > 1.0)
        return -1.f;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot(edge2, q);
    if (t > EPSILON) // ray intersection
    {
        return t;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return -1.f;
}

PrimaryHit GetIntersection(const Ray& ray, const Scene& scene, bool quitOnIntersect = false)
{
    PrimaryHit ret;

    for (const auto& model : scene.GetModels())
    {
        for (const auto& mesh : model.meshes)
        {
            // Iterate over each face
            for (size_t i = 0; i < mesh.faces.size(); i++)
            {
                const auto& face = mesh.faces[i];

                auto t = TriangleIntersect(
                    ray, face[0], face[1], face[2]);

                // Check hit
                if (t > 0.f && (t < ret.t || ret.t == -1.f))
                {
                    ret.isHit = true;

                    ret.t = t;
                    ret.model = &model;
                    ret.mesh = &mesh;

                    ret.hit = ray.origin + ray.dir * t;
                    ret.normal = mesh.normals[i];
                    
                    if (quitOnIntersect)
                    {
                        return ret;
                    }
                }
            }

        }
    }

    return ret;
}

bool IsOccluded(const Ray& ray, float dist2, const Model& targetModel, const Scene& scene)
{
    // Get closest intersection
    for (const auto& model : scene.GetModels())
    {
        if (&model == &targetModel)
            continue;

        for (const auto& mesh : model.meshes)
        {
            auto hit = GetIntersection(ray, scene);

            if (hit.t > 0.f)
            {
                const auto D = (ray.origin + ray.dir * hit.t) - ray.origin;
                const auto d2 = dot(D, D);
                if (d2 < dist2)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

float3 DirectIllumination(Xorshf96& rand, float3 hit, float3 normal, const Scene& scene)
{
    const auto& lights = scene.GetEmissive();
    const auto& light = lights[rand.random() % lights.size()];
    const float3 P = light->GetRandomPoint(rand.random());
    const float3 L = P - hit;
    const float3 dir = normalize(L);
    const float dist2 = dot(L, L);

    Ray shadow{ hit + dir * 0.0001f, dir };
    if (!IsOccluded(shadow, dist2, *light, scene))
    {
        float dotp = dot(normal, dir);
        auto color = ToColor(light->meshes.front().mat.color) * lights.size() * dotp;// / dist2;;
        return color;
    }
    return { 0 };
}

// I think the template optimizes the bool call since it generates a function definition
PrimaryHit Trace(Xorshf96& rand, const Ray& ray, const Scene& scene, bool quitOnIntersect = false)
{
    // Get closest intersection
    auto ret = GetIntersection(ray, scene);

    // No hit at all
    if (!ret.isHit)
    {
        ret.finalColor = ToPixel(ray.dir);
        return ret;
    }

    // Do shading
    float3 color;
    if (ret.mesh->mat.emissive)
    {
        ret.finalColor = ret.mesh->mat.color;
    }
    else
    {
        auto color = DirectIllumination(rand, ret.hit, ret.normal, scene);
        ret.finalColor = ToPixel(color * ret.mesh->mat.color);
    }
    
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
    Xorshf96& rand, Pixel* buffer, float3* accumelator, unsigned spp, float3 e, float3 topLeft, float3 right, float3 down,
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
            float px = 1.f / (float)bw;
            float py = 1.f / (float)bh;
            float3 r = u * right + px * rand.random(1.f);
            float3 d = v * down + py * rand.random(1.f);
            float3 P = topLeft + r + d;
            float3 D = normalize(P - e);

            Ray ray;
            ray.origin = e;
            ray.dir = D;

            auto hit = Trace(rand, ray, scene);
            
            accumelator[j * bw + i] += ToColor(hit.finalColor);
            float3 p = accumelator[j * bw + i];
            float scale = 1.0f / spp;
            p *= scale;
            
            buffer[j * bw + i] = ToPixel(p);
        }
    }
}

void Renderer::Init(Surface& screen, const Scene& scene, unsigned pixelCount, unsigned maxSampleCount)
{
    squareX = 16;
    squareY = 16;
    this->pixelCount = pixelCount;
    this->maxSampleCount = maxSampleCount;
    accumelator = std::make_unique<float3[]>(pixelCount);
    memset(accumelator.get(), 0, pixelCount * sizeof(float3));

    for (uint j = 0; j < screen.GetHeight() ; j += squareY)
    {
        for (uint i = 0; i < screen.GetWidth(); i += squareX)
        {
            AddTask( [&, i, j, r = Xorshf96(i + j * screen.GetWidth())]() mutable
            {
                RenderArea(r,
                    screen.GetBuffer(), 
                    accumelator.get(), spp, E, p0, right, down, i, j, squareX, squareY, 
                    screen.GetWidth(), screen.GetHeight(), scene);
            });
        }
    }
}

void Renderer::Render(const mat4& t, Surface& screen, const Scene& scene)
{
    if (spp >= maxSampleCount) { return; }
    //screen.Clear(0xAAAA00);
    spp++;

    // Calculate eye and screen
    p0 = t.TransformPoint(make_float3(-1, 1, 1)); // top-left
    p1 = t.TransformPoint(make_float3(1, 1, 1)); // top-right
    p2 = t.TransformPoint(make_float3(-1, -1, 1)); // bottom-left
    E = t.TransformPoint(make_float3(0, 0, 0));
    right = p1 - p0;
    down = p2 - p0;

    // If we want no MT :(
    //RenderArea(
    //    screen.GetBuffer(), accumelator.get(), spp, E, p0, right, down, 0, 0, width, height, width, height, scene);
    //return;

    // Calculate the tasks to render
    RunTasks();
    WaitForAll();
}

void Renderer::OnMove()
{
    spp = 0;
    memset(accumelator.get(), 0, pixelCount * sizeof(float3));
}

unsigned Renderer::SampleCount() const
{
    return spp;
}

unsigned Renderer::MaxSampleCount() const
{
    return maxSampleCount;
}
