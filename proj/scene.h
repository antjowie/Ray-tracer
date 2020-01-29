#pragma once

struct Primitive
{
    Primitive(float3 v0, float3 v1, float3 v2, uint c = 0xffffff) : color (c)
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

    void Clear();

    const std::vector<Primitive>& GetPrimitives() const;

private:
    std::vector<Primitive> m_primitives;
};