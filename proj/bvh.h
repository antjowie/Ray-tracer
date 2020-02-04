#pragma once

class Mesh;
class Model;

/**
 * A Model has a BVH, in the end they are merged
 *
 * It returns a Hit struct that contains the triangles/faces 
 * that are hit by the supplied ray
 */
class BVHAccelerator
{
public:
    struct Triangle
    {
        const Mesh* mesh;
        std::array<float3, 3> face;
        float3 normal;
    };

    struct Hit
    {
        int depth;
        // Is an array
        const Triangle* triangles;
        int count;
    };

    struct Node
    {
        float3 bmin;
        uint leftFirst;
        float3 bmax;
        uint count;
    };

    void Build(const Model& model);

    Hit Traverse(const Ray& ray) const;

private:

    Hit Traverse(const Node& node, const Ray& ray, int depth) const;

    std::vector<Triangle> triangles;
    //std::unique_ptr<BVHNode[]> tree;
    Node* tree = nullptr;
};