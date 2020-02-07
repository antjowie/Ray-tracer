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

const std::vector<const Model*> Scene::GetEmissive() const
{
    std::vector<const Model*> ret;
    for (const auto& model : m_models)
        for (const auto& mesh : model.meshes)
            if (mesh.mat.emissive)
            {
                ret.push_back(&model);
                break;
            }
            
    return ret;
}
