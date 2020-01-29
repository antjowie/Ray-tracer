#pragma once
/**
 * Contains the render systems that is responsible for rendering a scene
 */

// ------------
// Classes/Structs
// ------------
struct Ray
{
};

struct PrimaryHit
{
    float3 hit;
    float3 surfaceNormal;
};

// ------------
// Free functions
// ------------
void Render(const mat4& t, Surface& screen, );
void Trace(Ray s);
