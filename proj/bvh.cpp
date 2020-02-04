#include "precomp.h"

namespace {
    PrimaryHit TriangleIntersect(const Ray& ray, const float3& vertex0, const float3& vertex1, const float3& vertex2)
    {
        PrimaryHit hit;

        //https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        const float EPSILON = 0.0000001;
        float3 edge1, edge2, h, s, q;
        float a, f, u, v;
        edge1 = vertex1 - vertex0;
        edge2 = vertex2 - vertex0;
        h = cross(ray.dir, edge2);
        a = dot(edge1, h);
        if (a > -EPSILON && a < EPSILON)
            return hit;    // This ray is parallel to this triangle.
        f = 1.0 / a;
        s = ray.origin - vertex0;
        u = f * dot(s, h);
        if (u < 0.0 || u > 1.0)
            return hit;
        q = cross(s, edge1);
        v = f * dot(ray.dir, q);
        if (v < 0.0 || u + v > 1.0)
            return hit;
        // At this stage we can compute t to find out where the intersection point is on the line.
        float t = f * dot(edge2, q);
        if (t > EPSILON) // ray intersection
        {
            hit.isHit = true;
            hit.t = t;
            hit.hit = ray.origin + ray.dir * t;
            //TODO hit.surfaceNormal = 

            return hit;
        }
        else // This means that there is a line intersection but not a ray intersection.
            return hit;
    }
}

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
uint Split(const float3& bmin, const float3& bmax, std::vector<BVHAccelerator::Triangle>& triangles, uint begin, uint count)
{
    // For now, simple midsplit :)
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
    CalculateBounds(node.bmin, node.bmax, triangles, begin, count);
    if (count < 3)
    {
        node.leftFirst = begin;
        node.count = count;
        return;
    }

    // Reserve indices for self and right side

    auto splitOffset = Split(node.bmin,node.bmax,triangles, begin, count);
    auto left = index++;
    auto right = index++;
    node.leftFirst = left;
    node.count = 0;
    Subdivide(tree, tree[left], index, triangles, begin, splitOffset);
    Subdivide(tree, tree[right], index, triangles, begin + splitOffset, count - splitOffset);
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
    root.leftFirst = 1;
    root.count = 0;
    root.bmin = bmin;
    root.bmax = bmax;

    // Partition
    uint index = root.leftFirst;

    // Build children
    Subdivide(tree, tree[index], index, triangles, 0, triangles.size());
}

float AABBIntersect(const float3& bmin, const float3& bmax, const Ray& r)
{
    float tmin = (bmin.x - r.origin.x) / r.dir.x;
    float tmax = (bmax.x - r.origin.x) / r.dir.x;

    if (tmin > tmax) swap(tmin, tmax);

    float tymin = (bmin.y - r.origin.y) / r.dir.y;
    float tymax = (bmax.y - r.origin.y) / r.dir.y;

    if (tymin > tymax) swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return -1.f;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = (bmin.z - r.origin.z) / r.dir.z;
    float tzmax = (bmax.z - r.origin.z) / r.dir.z;

    if (tzmin > tzmax) swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return -1.f;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    if (tmin > 0.f) return tmin;
    else return -1.f;
}

BVHAccelerator::Hit BVHAccelerator::Traverse(const Ray& ray, PrimaryHit& pHit, bool quitOnIntersect) const
{
    auto& root = tree[tree[0].leftFirst];
    return Traverse(root, ray, 0,pHit, quitOnIntersect);
}

BVHAccelerator::Hit BVHAccelerator::Traverse(const Node& node, const Ray& ray, int depth, PrimaryHit& pHit, bool quitOnIntersect) const
{
    Hit hit;
    hit.count = 0;
    hit.t = -1.f;
    hit.depth = depth;
    hit.triangles = &triangles[node.leftFirst];
    
    // Check if it intersects with this node
    hit.t = AABBIntersect(node.bmin, node.bmax, ray);
    if (hit.t == -1.f)
    {
        return hit;
    }

    // Check if it should return the triangles
    if (node.count != 0)
    {
        return hit;
        //IntersectTriangles();
    }

    // Continue walking the tree
    auto& left = tree[node.leftFirst];
    auto& right = tree[node.leftFirst + 1];

    auto h1 = Traverse(left, ray, depth + 1, pHit);
    auto h2 = Traverse(right, ray, depth + 1, pHit);
    hit.depth += h1.depth + h2.depth;
    h1.depth = h2.depth = hit.depth;
    if (h1.t <= 0.f && h2.t <= 0.f)
    {
        hit.t = -1.f;
        return hit;
    }
    if (h1.t > 0.f && h2.t > 0.f)
    {
        if (h1.t < h2.t)
            return h1;
        else
            return h2;
    }
    if (h1.t > 0.f)
        return h1;
    if (h2.t > 0.f)
        return h2;
}