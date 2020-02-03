#pragma once

/**
 * Data structure for models and meshes
 */

struct Material
{
    Pixel color;
};

struct Mesh
{
    std::vector<unsigned> indices; // points to 1 vertex, so 3 elements
    std::vector<float> vertices; // x y z

    std::vector<float> faces; // Saved for only one vertex in the face

    aabb bb; // Local space
    Material mat;
};

struct Model
{
    mat4 transform = mat4::Identity();
    std::vector<Mesh> meshes;
};

struct PointLight
{
    float3 pos;
    float intensity;

    Pixel color = 0xFFFFFFFF;
};