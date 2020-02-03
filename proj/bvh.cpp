#include "precomp.h"
#include <cstdlib>

// Calculates the bounds that the triangles range contains
void CalculateBounds(float3& bmin, float3& bmax, std::vector<BVHAccelerator::Triangle>& triangles, uint begin, uint count)
{
    bmin = make_float3(std::numeric_limits<float>::max());
    bmax = make_float3(std::numeric_limits<float>::min());

    for (size_t i = begin; i < count; i++)
    {
        auto& tri = triangles[i];

        for (int i = 0; i < 3; i++)
        {
            bmin = fminf(bmin, tri.face[i]);
            bmax = fmaxf(bmax, tri.face[i]);
        }
    }
}

// Returns the offset to split, sorts the view that it receives
uint Split(std::vector<BVHAccelerator::Triangle>& triangles, uint begin, uint count)
{
    // For now, simple midsplit :)
    float3 bmin, bmax;
    CalculateBounds(bmin, bmax, triangles, begin, count);

    // Get largest axis
    float3 length = bmax - bmin;

    int axis = 0;
    float longest = length.x;
    if (length.y > longest) { longest = length.y; axis = 1; };
    if (length.z > longest) { longest = length.z; axis = 2; };

    std::sort(triangles.begin() + begin, triangles.begin() + begin + count, 
        [axis](const BVHAccelerator::Triangle& t1, const BVHAccelerator::Triangle& t2)
    {
        // Get center
        float min1 = std::numeric_limits<float>::max();
        float min2 = std::numeric_limits<float>::max();
        
        for (auto& face : t1.face)
        { min1 = face.m[axis] < min1 ? face.m[axis] : min1; }

        for (auto& face : t2.face)
        { min2 = face.m[axis] < min2 ? face.m[axis] : min2; }
        return min1 < min2;
    });

    return count / 2;
}

// Divide according to a criteria
// node should be the next node
void Subdivide(BVHAccelerator::BVHNode* tree, BVHAccelerator::BVHNode& node, uint& index, std::vector<BVHAccelerator::Triangle>& triangles, uint begin, uint count)
{
    // If we are looking at 3 triangles, we will just refer to them
    if (count < 3)
    {
        CalculateBounds(node.bmin, node.bmax, triangles, begin, count);
        node.leftFirst = begin;
        node.count = count;
        return;
    }

    // Reserve indices for self and right side
    index += 2;
    node.count = 0;

    auto splitOffset = Split(triangles, begin, count);
    auto right = index - 1;
    Subdivide(tree, tree[index], index, triangles, begin, splitOffset);
    Subdivide(tree, tree[right], index, triangles, begin + splitOffset, begin + count);
}


void BVHAccelerator::Build(const Model& model)
{
    if (tree) delete[] tree;

    // Create root node
    // While filling, calculate root bounds
    float3 bmin = make_float3(std::numeric_limits<float>::max());
    float3 bmax = make_float3(std::numeric_limits<float>::min());
    for (const auto& mesh : model.meshes)
    {
        for (const auto& face : mesh.faces)
        {
            auto& tri = triangles.emplace_back();
            tri.mesh = &mesh;
            tri.face = face;

            for (int i = 0; i < 3; i++)
            {
                bmin = fminf(bmin, tri.face[i]);
                bmax = fmaxf(bmax, tri.face[i]);
            }
        }
    }

    tree = new BVHNode[triangles.size() * 2];

    auto& root = tree[0];
    root.leftFirst = 2;
    root.count = 0;
    root.bmin = bmin;
    root.bmax = bmax;

    // Partition
    auto index = root.leftFirst;

    // Build children
    Subdivide(tree, tree[index], index, triangles, 0, triangles.size());
}