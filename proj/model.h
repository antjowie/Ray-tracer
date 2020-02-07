#pragma once

/**
 * Data structure for models and meshes
 */

struct Material
{
    Pixel color = 0xAA00AA;

    bool emissive = false;
};

struct Mesh
{
    std::vector<std::array<float3,3>> faces;
    std::vector<float3> normals;

    Material mat;

    float3 GetRandomPoint(unsigned random) const;
};

struct Model
{
    mat4 transform = mat4::Identity();
    std::vector<Mesh> meshes;

    // Gets a random point in one of the meshes
    // Pass in index since rand is not guranteed to be thread safe
    float3 GetRandomPoint(unsigned random) const;
};

struct PointLight
{
    float3 pos;
    float intensity;

    Pixel color = 0xFFFFFFFF;
};