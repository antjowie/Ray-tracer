#pragma once

struct Primitive
{
    Primitive(const float3 v0, const float3 v1, const float3 v2, uint c = 0xffffff) : color (c)
    { v[0] = v0; v[1] = v1; v[2] = v2; }
    
    uint color;
    float3 v[3];
};

/**
 * Responsible for ownership of meshes
 */
class Scene
{
public:
    void Add(Primitive&& primitive);
    void Add(const char* path); // Loads a gltf file

    void Clear();

    const std::vector<Primitive>& GetPrimitives() const;

private:
    std::vector<Primitive> m_primitives;
};