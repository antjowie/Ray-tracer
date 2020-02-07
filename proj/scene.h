#pragma once

/**
 * Responsible for ownership of meshes
 */
class Scene
{
public:
    void Add(Model&& model);
    
    void Clear();

    const std::vector<Model>& GetModels() const;

    // NOTE: If part of a mesh is emissive, the whole model is seen as a light
    // This is temp for simplicity
    const std::vector<const Model*> GetEmissive() const;

private:
    std::vector<Model> m_models;
};