#pragma once

/**
 * A Model has a BVH, in the end they are merged
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

    void Build(const Model& model);

    Hit Traverse(const Ray& ray);

private:

    struct Hit
    {
        float t;
        Triangle* triangle;
    };

    Hit Traverse(const Node& node, const Ray& ray);

    std::vector<Triangle> triangles;
    //std::unique_ptr<BVHNode[]> tree;
    Node* tree = nullptr;
};