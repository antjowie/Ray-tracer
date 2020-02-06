#pragma once
/**
 * Contains the render systems that is responsible for rendering a scene
 */

// ------------
// Classes/Structs
// ------------
struct Ray
{
    float3 origin;
    float3 dir;
};

struct PrimaryHit
{
    bool isHit = false;
    float t = -1.f;

    const Model* model;
    const Mesh* mesh;
    float3 hit;
    float3 normal;
    
    Pixel color;
};

class Renderer
{
public:
    void Init(Surface& screen, const Scene& scene, unsigned pixelCount, unsigned maxSampleCount = 128);

    void Render(const mat4& t, Surface& screen, const Scene& scene);

    // Used to reset renderer state
    void OnMove();

    unsigned SampleCount() const;
    unsigned MaxSampleCount() const;
    
    unsigned squareX;
    unsigned squareY;


private:
    unsigned spp = 0;
    unsigned maxSampleCount;
    unsigned pixelCount;

    float3 p0;
    float3 p1;
    float3 p2;
    float3 E;
    float3 right;
    float3 down;

    std::unique_ptr<float3[]> accumelator;
};
