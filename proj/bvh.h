#pragma once
#include <raytracer.h>

/**
 * A Model has a BVH, in the end they are merged
 */
class BVHAccelerator
{
public:
    struct Triangle
    {
        const Mesh* mesh;
        std::array<float3,3> face;
    };

    struct BVHNode
    {
        float3 bmin;
        uint leftFirst;
        float3 bmax;
        uint count;
    };

    void Build(const Model& model);

    PrimaryHit Traverse(const Ray& ray);

private:
    std::vector<Triangle> triangles;
    //std::unique_ptr<BVHNode[]> tree;
    BVHNode* tree = nullptr;
};