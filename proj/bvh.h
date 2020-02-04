#pragma once

class Mesh;

/**
 * A Model has a BVH, in the end they are merged
 *
 * It returns a Hit struct that contains the triangles/faces 
 * that are hit by the supplied ray
 */
class BVHAccelerator
{
private:
    struct Hit;

public:
    struct Triangle
    {
        const Mesh* mesh;
        std::array<float3,3> face;
        float3 normal;
    };

    struct Node
    {
        float3 bmin;
        uint leftFirst;
        float3 bmax;
        uint count;
    };

    // Since we get circular dependency, we just pass pointer
    void Build(const Model* const model);

    Hit Traverse(const Ray& ray);

private:

    struct Hit
    {
        float t;
        // Is an array
        Triangle* triangles; 
        int count;
    };

    Hit Traverse(const Node& node, const Ray& ray);

    std::vector<Triangle> triangles;
    //std::unique_ptr<BVHNode[]> tree;
    Node* tree = nullptr;
};