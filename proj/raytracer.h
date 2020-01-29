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
    float3 hit;
    float3 surfaceNormal;
};

// ------------
// Free functions
// ------------
void Render(const mat4& t, Surface& screen, const Scene& scene);
float3 Trace(Ray ray, const Scene& scene);
