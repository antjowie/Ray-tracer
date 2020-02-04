#pragma once

class Mesh;
class Model;

struct PrimaryHit
{
    bool isHit = false;
    float t = 0.f;

    const Model* model;
    const Mesh* mesh;
    float3 hit;
    float3 surfaceNormal;

    Pixel color;
};

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
        float t;
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

    Hit Traverse(const Ray& ray, PrimaryHit& pHit, bool quitOnIntersect = false) const;

private:

    Hit Traverse(const Node& node, const Ray& ray, int depth, PrimaryHit& pHit, bool quitOnIntersect = false) const;

    std::vector<Triangle> triangles;
    //std::unique_ptr<BVHNode[]> tree;
    Node* tree = nullptr;
};