#include "precomp.h"

float3 Mesh::GetRandomPoint(unsigned random) const
{
    // NOTE: This function takes a random point in a random face of the mesh
    // It is probably not uniform.

    const auto& face = faces[random % faces.size()];

    float3 A = face[0];
    float3 B = face[1];
    float3 C = face[2];

    float3 BA = B - A;
    float3 CA = C - A;

    float u = Rand(1.f);
    float v = Rand(1.f);

    // To stay inside of domain we flip it
    if (u + v > 1.f)
    {
        u = 1.f - u;
        v = 1.f - v;
    }

    return A + (BA * u) + (CA * v);
}

float3 Model::GetRandomPoint(unsigned random) const
{
    return meshes[random % meshes.size()].GetRandomPoint(random);
}
