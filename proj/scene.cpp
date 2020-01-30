#include "precomp.h"

void Scene::Add(Model&& model)
{
    m_models.push_back(model);
}

void Scene::Clear()
{
    m_models.clear();
}

const std::vector<Model>& Scene::GetModels() const
{
    return m_models;
}
