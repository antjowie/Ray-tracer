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

private:
    std::vector<Model> m_models;
};