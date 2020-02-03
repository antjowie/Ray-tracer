#include "precomp.h"
#include <cstdlib>

void BVHAccelerator::Build(const Model& model)
{

    // Get all primitives
    float3 min = make_float3(std::numeric_limits<float>::max());
    float3 max = make_float3(std::numeric_limits<float>::min());
    
    for (const auto& mesh : model.meshes)
    {
    }

    // Calculate world bounds


    // Use split criteria

    // Build left most till leaf

    // Repeat
}