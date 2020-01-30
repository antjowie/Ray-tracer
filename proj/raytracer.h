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
    float t = 0.f;

    const Model* model;
    const Mesh* mesh;
    float3 hit;
    float3 surfaceNormal;
};

class Renderer
{
public:
    void Render(const mat4& t, Surface& screen, const Scene& scene);

    unsigned threadCount;
    unsigned squareX;
    unsigned squareY;
};
