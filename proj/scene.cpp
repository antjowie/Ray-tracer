#include "precomp.h"

void Scene::Add(Primitive&& primitive)
{
    m_primitives.push_back(primitive);
}

void Scene::Clear()
{
    m_primitives.clear();
}

const std::vector<Primitive>& Scene::GetPrimitives() const
{
    return m_primitives;
}
