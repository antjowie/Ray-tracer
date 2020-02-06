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

struct TraceHit
{
    float t = -1.f;
    float3 normal;
};

TraceHit GetIntersection(const Ray& ray, const Mesh& mesh, bool quitOnIntersect = false)
{
    TraceHit ret;

    // Iterate over each face
    for (size_t i = 0; i < mesh.faces.size(); i++)
    {
        const auto& face = mesh.faces[i];

        auto t = TriangleIntersect(
            ray, face[0], face[1], face[2]);

        // Check hit
        if (t > 0.f && (t < ret.t || ret.t == -1.f))
        {
            ret.t = t;
            ret.normal = mesh.normals[i];

            if (quitOnIntersect)
            {
                return ret;
            }
        }
    }

    return ret;
}

// I think the template optimizes the bool call since it generates a function definition
PrimaryHit Trace(const Ray& ray, const Scene& scene, bool quitOnIntersect = false)
{
    PrimaryHit ret;

    // Get closest intersection
    for (const auto& model : scene.GetModels())
    {        
        for (const auto& mesh : model.meshes)
        {
            auto hit = GetIntersection(ray, mesh, quitOnIntersect);

            if (hit.t > 0.f && (ret.t == -1.f || hit.t < ret.t))
            {
                ret.isHit = true;

                ret.t = hit.t;
                ret.model = &model;
                ret.mesh = &mesh;

                ret.hit = ray.origin + ray.dir * hit.t;
                ret.normal = hit.normal;

                if (quitOnIntersect)
                {
                    ret.color = ToPixel(ray.dir * 0xff);
                    return ret;
                }
            }
        }
    }

    // No hit at all
    if (!ret.isHit)
    {
        ret.color = ToPixel(ray.dir);
        return ret;
    }

    // Do whitted shading
    float3 finalColor;
    for(const auto& light: scene.GetLights())
    {
        float3 dir = normalize(light.pos - ret.hit);
        Ray shadow{ ret.hit + dir*0.0001f, dir };

        // Pronounced as s-hit
        auto shit = Trace(shadow, scene, true);
        if(!shit.isHit)
        {
            // This is very incorrect but temp
            float l = 1.f; // Light intensity

            finalColor += ToColor(ret.mesh->mat.color) * l * max(0.f,dot(ret.normal,shadow.dir));
        }
    }

    
    ret.color = ToPixel(finalColor);
    
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

            auto hit = Trace(ray, scene);
            
            accumelator[j * bw + i] += ToColor(hit.color);
            float3 p = accumelator[j * bw + i];
            float scale = 1.0f / spp;
            p *= scale;
            
            buffer[j * bw + i] = ToPixel(p);
        }
    }
    //std::cout << rand.random(1.f) << '\n';
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
                
                //(i + j * screen.GetWidth());
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
