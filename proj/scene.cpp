#include "precomp.h"

void Scene::Add(Primitive&& primitive)
{
    m_primitives.push_back(primitive);
}

void Scene::Add(const char* path)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    // Parse gltf
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse glTF\n");
        return;
    }

    std::cout << "loaded glTF file has:\n"
        << model.accessors.size() << " accessors\n"
        << model.animations.size() << " animations\n"
        << model.buffers.size() << " buffers\n"
        << model.bufferViews.size() << " bufferViews\n"
        << model.materials.size() << " materials\n"
        << model.meshes.size() << " meshes\n"
        << model.nodes.size() << " nodes\n"
        << model.textures.size() << " textures\n"
        << model.images.size() << " images\n"
        << model.skins.size() << " skins\n"
        << model.samplers.size() << " samplers\n"
        << model.cameras.size() << " cameras\n"
        << model.scenes.size() << " scenes\n"
        << model.lights.size() << " lights\n";


}

void Scene::Clear()
{
    m_primitives.clear();
}

const std::vector<Primitive>& Scene::GetPrimitives() const
{
    return m_primitives;
}
