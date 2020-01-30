#pragma once

/**
 * Data structure for models and meshes
 */

struct Mesh
{
    std::vector<float> vertices;
    std::vector<unsigned> indices;
    Pixel color;
};

struct Model
{
    mat4 transform = mat4::Identity();
    std::vector<Mesh> meshes;
};