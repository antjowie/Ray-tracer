#pragma once

class Scene;

class BVHAccelerator
{
public:
    void Build(const Model& model);

    //PrimaryHit Traverse(const Ray& ray);

private:
    struct Triangle
    {
        const Mesh* mesh;
        float3 triangle[3];
    };

    struct BVHNode
    {
        float3 bmin;
        int *leftFirst;
        float3 bmax;
        int count;
    };

    std::vector<Triangle> triangles;
    //std::unique_ptr<BVHNode[]> tree;
    BVHNode* tree;
};