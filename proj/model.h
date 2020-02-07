#pragma once

/**
 * Data structure for models and meshes
 */

struct Material
{
    Pixel color;

    bool emissive;
};

struct Mesh
{
    std::vector<std::array<float3,3>> faces;
    std::vector<float3> normals;

    Material mat;

    float3 GetRandomPoint() const;
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