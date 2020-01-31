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
            // i is the index that we currently are at
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


                        ret.surfaceNormal = make_float3(
                                mesh.faces[i / 3 + 0],
                                mesh.faces[i / 3 + 1],
                                mesh.faces[i / 3 + 2]
                            );
                    }
                }
            }
        }
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
    Pixel* buffer, float3 e, float3 topLeft, float3 right, float3 down,
    uint x, uint y, uint w, uint h, uint bw, uint bh, const Scene& scene, int index)
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
            Pixel c;

            if (hit.isHit)
            {
                c = hit.mesh->mat.color;
                //auto col = lerp(make_float3(0), make_float3(0xff), dot(ray.dir * -1.f, hit.surfaceNormal));
                //c = ((static_cast<uint>(std::fabsf(col.x)) & 0xFF)) |
                //    ((static_cast<uint>(std::fabsf(col.y)) & 0xFF) << 8) |
                //    ((static_cast<uint>(std::fabsf(col.z)) & 0xFF) << 16);
            }
            else
            {
                float3 normalizedColor = ray.dir * 0xff;
                c = ((static_cast<uint>(std::fabsf(normalizedColor.x)) & 0xFF)) |
                    ((static_cast<uint>(std::fabsf(normalizedColor.y)) & 0xFF) << 8) |
                    ((static_cast<uint>(std::fabsf(normalizedColor.z)) & 0xFF) << 16);
            }

            
            // Set color value
            buffer[j * bw + i] = c;
            //buffer[i * bw + j] = ScaleColor(c,dot(ray.dir,hit.surfaceNormal) * 0xff);
        }
    }
    std::cout << "Index done: " << index << '\n';
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

    //AddTask([&]()
    //{
    //    RenderArea(screen.GetBuffer(), E, p0, right, down, 
    //        0, 0, 
    //        width/2, height, 
    //        width, height, scene);
    //});
    //AddTask([&]()
    //{
    //    RenderArea(screen.GetBuffer(), E, p0, right, down, 
    //        width/2, 0, 
    //        width, height, 
    //        width, height, scene);
    //});

    static int a = 0;
    // Calculate the tasks to render
    for (uint j = 0; j < height - squareY; j += squareY)
    {
        for (uint i = 0; i < width - squareX; i += squareX)
        {
            AddTask([&]()
            {
                RenderArea(
                    screen.GetBuffer(), E, p0, right, down, i, j, squareX, squareY, width, height, scene, a);
            });
            a++;
        }
    }
    a = 0;
    
    //for (uint j = 0; j < height - 1; j++)
    //{
    //    for (uint i = 0; i < width - 1; i++)
    //    {
    //        AddTask([&]()
    //        {
    //            RenderArea(
    //                screen.GetBuffer(), E, p0, right, down, i, j, 1, 1, width, height, scene);
    //        });
    //    }
    //}
    

    RunTasks();
    WaitForAll();

    taskflow.clear();
    //std::cout << screen.GetBuffer()[0] << ' ' << screen.GetBuffer()[512 * 512 - 1] << '\n';
    //// Calculate ray directions
    //RenderArea(
    //    screen.GetBuffer(), E, p0, right, down, 
    //    0, 0, screen.GetWidth(), screen.GetHeight(), screen.GetWidth(), screen.GetHeight(), scene);
 }