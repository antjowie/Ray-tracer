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
    std::vector<float> vertices;
    std::vector<unsigned> indices;

    std::vector<float> faces;

    aabb bb; // Local space
    Material mat;
};

struct Model
{
    mat4 transform = mat4::Identity();
    std::vector<Mesh> meshes;
};