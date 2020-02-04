#include "precomp.h"

void Scene::Add(Model&& model)
{
    auto& m = m_models.emplace_back(model);
    m.bvh.Build(m);
}

void Scene::Add(PointLight&& light)
{
    m_lights.push_back(light);
}

void Scene::Clear()
{
    m_models.clear();
}

const std::vector<Model>& Scene::GetModels() const
{
    return m_models;
}

const std::vector<PointLight>& Scene::GetLights() const
{
    return m_lights;
}
