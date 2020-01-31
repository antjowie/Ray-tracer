#pragma once

/**
 * Responsible for ownership of meshes
 */
class Scene
{
public:
    void Add(Model&& model);
    void Add(PointLight&& light);

    void Clear();

    const std::vector<Model>& GetModels() const;
    const std::vector<PointLight>& GetLights() const;

private:
    std::vector<Model> m_models;
    std::vector<PointLight> m_lights;
};