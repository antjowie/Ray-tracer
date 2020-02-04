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
    std::vector<std::array<float3,3>> faces; // x y z
    std::vector<float3> normals; // Saved for only one vertex in the face

    Material mat;
};

struct Model
{
    mat4 transform = mat4::Identity();
    std::vector<Mesh> meshes;
    BVHAccelerator bvh;
};

struct PointLight
{
    float3 pos;
    float intensity;

    Pixel color = 0xFFFFFFFF;
};