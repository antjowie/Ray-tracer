#include "precomp.h"

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
void Subdivide(BVHAccelerator::Node* tree, BVHAccelerator::Node& node, uint& index, std::vector<BVHAccelerator::Triangle>& triangles, uint begin, uint count)
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
    Subdivide(tree, tree[right], index, triangles, begin + splitOffset, splitOffset);
}


void BVHAccelerator::Build(const Model* const model)
{
    if (tree) delete[] tree;

    // Create root node
    // While filling, calculate root bounds
    float3 bmin = make_float3(std::numeric_limits<float>::max());
    float3 bmax = make_float3(std::numeric_limits<float>::min());
    for (const auto& mesh : model->meshes)
    {
        for (int i = 0; i < mesh.faces.size(); i++)
        {
            auto& tri = triangles.emplace_back();
            tri.mesh = &mesh;
            tri.face = mesh.faces[i];
            tri.normal = mesh.normals[i];

            for (int i = 0; i < 3; i++)
            {
                bmin = fminf(bmin, tri.face[i]);
                bmax = fmaxf(bmax, tri.face[i]);
            }
        }
    }

    tree = new Node[triangles.size() * 2];

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

float AABBIntersect(const float3& bmin, const float3& bmax, const Ray& r)
{
    float tx1 = (bmin.x - r.origin.x) * r.dir.x;
    float tx2 = (bmax.x - r.origin.x) * r.dir.x;
    float tmin = min(tx1, tx2);
    float tmax = max(tx1, tx2);
    float ty1 = (bmin.y - r.origin.y) * r.dir.y;
    float ty2 = (bmax.y - r.origin.y) * r.dir.y;
    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));
    float tz1 = (bmin.z - r.origin.z) * r.dir.z;
    float tz2 = (bmax.z - r.origin.z) * r.dir.z;
    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));
    return (tmax >= tmin && tmax >= 0.f) ? tmin : -1.f;
}

//bool AABBIntersect(box b, ray r)
//{
//    __m128 t1 = _mm_mul_ps(_mm_sub_ps(node->bmin4, O4), rD4);
//    __m128 t2 = _mm_mul_ps(_mm_sub_ps(node->bmax4, O4), rD4);
//    __m128 vmax4 = _mm_max_ps(t1, t2), vmin4 = _mm_min_ps(t1, t2);
//    float* vmax = (float*)&vmax4, * vmin = (float*)&vmin4;
//    float tmax = min(vmax[0], min(vmax[1], vmax[2]));
//    float tmin = max(vmin[0], max(vmin[1], vmin[2]));
//    return tmax >= tmin && tmax >= 0;
//}

BVHAccelerator::Hit BVHAccelerator::Traverse(const Ray& ray)
{
    auto& root = tree[2];
    return Traverse(root, ray);
}

BVHAccelerator::Hit BVHAccelerator::Traverse(const Node& node, const Ray& ray)
{
    Hit hit;
    // Check if it should return the triangles
    if (node.count != 0)
    {
        hit.count = node.count;
        hit.triangles = &triangles[node.leftFirst];
        return hit;
    }

    // Continue walking the tree
    auto& left = tree[node.leftFirst];
    auto& right = tree[node.leftFirst + 1];

    auto f1 = AABBIntersect(left.bmin, left.bmax, ray);
    auto f2 = AABBIntersect(right.bmin, right.bmax, ray);

    if (f1 < f2) // If left is closer
    {
        auto hit = Traverse(left, ray);
        if (hit.count != 0) // Any hit?
            return Traverse(right, ray);
    }
    else
    {
        auto hit = Traverse(right, ray);
        if (hit.count != 0) // Any hit?
            return Traverse(left, ray);
    }
}