#include "precomp.h"

// https://github.com/syoyo/tinygltf/blob/master/examples/raytrace/gltf-loader.h
namespace
{
    /// Adapts an array of bytes to an array of T. Will advace of byte_stride each
/// elements.
    template <typename T>
    struct arrayAdapter {
        /// Pointer to the bytes
        const unsigned char* dataPtr;
        /// Number of elements in the array
        const size_t elemCount;
        /// Stride in bytes between two elements
        const size_t stride;

        /// Construct an array adapter.
        /// \param ptr Pointer to the start of the data, with offset applied
        /// \param count Number of elements in the array
        /// \param byte_stride Stride betweens elements in the array
        arrayAdapter(const unsigned char* ptr, size_t count, size_t byte_stride)
            : dataPtr(ptr), elemCount(count), stride(byte_stride) {}

        /// Returns a *copy* of a single element. Can't be used to modify it.
        T operator[](size_t pos) const {
            if (pos >= elemCount)
                throw std::out_of_range(
                    "Tried to access beyond the last element of an array adapter with "
                    "count " +
                    std::to_string(elemCount) + " while getting elemnet number " +
                    std::to_string(pos));
            return *(reinterpret_cast<const T*>(dataPtr + pos * stride));
        }
    };

    /// Interface of any adapted array that returns ingeger data
    struct intArrayBase {
        virtual ~intArrayBase() = default;
        virtual unsigned int operator[](size_t) const = 0;
        virtual size_t size() const = 0;
    };

    /// Interface of any adapted array that returns float data
    struct floatArrayBase {
        virtual ~floatArrayBase() = default;
        virtual float operator[](size_t) const = 0;
        virtual size_t size() const = 0;
    };

    /// An array that loads interger types, returns them as int
    template <class T>
    struct intArray : public intArrayBase {
        arrayAdapter<T> adapter;

        intArray(const arrayAdapter<T>& a) : adapter(a) {}
        unsigned int operator[](size_t position) const override {
            return static_cast<unsigned int>(adapter[position]);
        }

        size_t size() const override { return adapter.elemCount; }
    };

    template <class T>
    struct floatArray : public floatArrayBase {
        arrayAdapter<T> adapter;

        floatArray(const arrayAdapter<T>& a) : adapter(a) {}
        float operator[](size_t position) const override {
            return static_cast<float>(adapter[position]);
        }

        size_t size() const override { return adapter.elemCount; }
    };



#pragma pack(push, 1)

    template <typename T>
    struct v2 {
        T x, y;
    };
    /// 3D vector of floats without padding
    template <typename T>
    struct v3 {
        T x, y, z;
    };

    /// 4D vector of floats without padding
    template <typename T>
    struct v4 {
        T x, y, z, w;
    };

#pragma pack(pop)

    using v2f = v2<float>;
    using v3f = v3<float>;
    using v4f = v4<float>;
    using v2d = v2<double>;
    using v3d = v3<double>;
    using v4d = v4<double>;

    struct v2fArray {
        arrayAdapter<v2f> adapter;
        v2fArray(const arrayAdapter<v2f>& a) : adapter(a) {}

        v2f operator[](size_t position) const { return adapter[position]; }
        size_t size() const { return adapter.elemCount; }
    };

    struct v3fArray {
        arrayAdapter<v3f> adapter;
        v3fArray(const arrayAdapter<v3f>& a) : adapter(a) {}

        v3f operator[](size_t position) const { return adapter[position]; }
        size_t size() const { return adapter.elemCount; }
    };

    struct v4fArray {
        arrayAdapter<v4f> adapter;
        v4fArray(const arrayAdapter<v4f>& a) : adapter(a) {}

        v4f operator[](size_t position) const { return adapter[position]; }
        size_t size() const { return adapter.elemCount; }
    };

    struct v2dArray {
        arrayAdapter<v2d> adapter;
        v2dArray(const arrayAdapter<v2d>& a) : adapter(a) {}

        v2d operator[](size_t position) const { return adapter[position]; }
        size_t size() const { return adapter.elemCount; }
    };

    struct v3dArray {
        arrayAdapter<v3d> adapter;
        v3dArray(const arrayAdapter<v3d>& a) : adapter(a) {}

        v3d operator[](size_t position) const { return adapter[position]; }
        size_t size() const { return adapter.elemCount; }
    };

    struct v4dArray {
        arrayAdapter<v4d> adapter;
        v4dArray(const arrayAdapter<v4d>& a) : adapter(a) {}

        v4d operator[](size_t position) const { return adapter[position]; }
        size_t size() const { return adapter.elemCount; }
    };
}

void Scene::Add(Primitive&& primitive)
{
    m_primitives.push_back(primitive);
}

void Scene::Add(const char* path)
{
    //https://github.com/syoyo/tinygltf/blob/master/examples/raytrace/gltf-loader.cc
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

    float3 pMin = {}, pMax = {};

    for (const auto& mesh : model.meshes)
    {
        std::cout << "Loading mesh: " << mesh.name << '\n';

        for (const auto& prim : mesh.primitives)
        {
            // Boolean used to check if we have converted the vertex buffer format
            bool convertedToTriangleList = false;
            // This permit to get a type agnostic way of reading the index buffer
            std::unique_ptr<intArrayBase> indicesArrayPtr = nullptr;
            {
                const auto& indicesAccessor = model.accessors[prim.indices];
                const auto& bufferView = model.bufferViews[indicesAccessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const auto dataAddress = buffer.data.data() + bufferView.byteOffset +
                    indicesAccessor.byteOffset;
                const auto byteStride = indicesAccessor.ByteStride(bufferView);
                const auto count = indicesAccessor.count;

                // Convert indices from buffer to readable data
                // Allocate the index array in the pointer-to-base declared in the
                // parent scope
                switch (indicesAccessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_BYTE:
                    indicesArrayPtr =
                        std::unique_ptr<intArray<char> >(new intArray<char>(
                            arrayAdapter<char>(dataAddress, count, byteStride)));
                    break;

                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    indicesArrayPtr = std::unique_ptr<intArray<unsigned char> >(
                        new intArray<unsigned char>(arrayAdapter<unsigned char>(
                            dataAddress, count, byteStride)));
                    break;

                case TINYGLTF_COMPONENT_TYPE_SHORT:
                    indicesArrayPtr =
                        std::unique_ptr<intArray<short> >(new intArray<short>(
                            arrayAdapter<short>(dataAddress, count, byteStride)));
                    break;

                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    indicesArrayPtr = std::unique_ptr<intArray<unsigned short> >(
                        new intArray<unsigned short>(arrayAdapter<unsigned short>(
                            dataAddress, count, byteStride)));
                    break;

                case TINYGLTF_COMPONENT_TYPE_INT:
                    indicesArrayPtr = std::unique_ptr<intArray<int> >(new intArray<int>(
                        arrayAdapter<int>(dataAddress, count, byteStride)));
                    break;

                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    indicesArrayPtr = std::unique_ptr<intArray<unsigned int> >(
                        new intArray<unsigned int>(arrayAdapter<unsigned int>(
                            dataAddress, count, byteStride)));
                    break;
                default:
                    break;
                }

                const auto& indices = *indicesArrayPtr;

                if (indicesArrayPtr) {
                    std::cout << "indices: ";
                    for (size_t i(0); i < indicesArrayPtr->size(); ++i) {
                        std::cout << indices[i] << " ";
                        //loadedMesh.faces.push_back(indices[i]);
                    }
                }
                std::cout << '\n';

                // Get the correct mode and index with indices
                switch (prim.mode) {
                case TINYGLTF_MODE_TRIANGLES:  // this is the simpliest case to handle
                {
                    for (const auto& attribute : prim.attributes) {
                        const auto attribAccessor = model.accessors[attribute.second];
                        const auto& bufferView =
                            model.bufferViews[attribAccessor.bufferView];
                        const auto& buffer = model.buffers[bufferView.buffer];
                        const auto dataPtr = buffer.data.data() + bufferView.byteOffset +
                            attribAccessor.byteOffset;
                        const auto byte_stride = attribAccessor.ByteStride(bufferView);
                        const auto count = attribAccessor.count;

                        std::cout << "current attribute has count " << count
                            << " and stride " << byte_stride << " bytes\n";

                        std::cout << "attribute string is : " << attribute.first << '\n';
                        if (attribute.first == "POSITION") {
                            std::cout << "found position attribute\n";

                            // get the position min/max for computing the boundingbox
                            pMin.x = attribAccessor.minValues[0];
                            pMin.y = attribAccessor.minValues[1];
                            pMin.z = attribAccessor.minValues[2];
                            pMax.x = attribAccessor.maxValues[0];
                            pMax.y = attribAccessor.maxValues[1];
                            pMax.z = attribAccessor.maxValues[2];

                            switch (attribAccessor.type) {
                            case TINYGLTF_TYPE_VEC3: {
                                switch (attribAccessor.componentType) {
                                case TINYGLTF_COMPONENT_TYPE_FLOAT:
                                    std::cout << "Type is FLOAT\n";
                                    // 3D vector of float
                                    v3fArray positions(
                                        arrayAdapter<v3f>(dataPtr, count, byte_stride));

                                    std::cout << "positions's size : " << positions.size()
                                        << '\n';

                                    for (size_t i{ 0 }; i < positions.size(); i =  i + 3) {
                                        const auto v0 = positions[i + 0];
                                        const auto v1 = positions[i + 1];
                                        const auto v2 = positions[i + 2];
                                        std::cout << "positions[" << i + 0 << "]: (" << v0.x << ", "
                                            << v0.y << ", " << v0.z << ")\n";
                                        std::cout << "positions[" << i + 1 << "]: (" << v1.x << ", "
                                            << v1.y << ", " << v1.z << ")\n";
                                        std::cout << "positions[" << i + 2 << "]: (" << v2.x << ", "
                                            << v2.y << ", " << v2.z << ")\n";

                                        // Push primitive triangle in scene
                                        Add(Primitive(
                                            make_float3(v0.x, v0.y, v0.z),
                                            make_float3(v1.x, v1.y, v1.z),
                                            make_float3(v2.x, v2.y, v2.z)));
                                        //loadedMesh.vertices.push_back(v.x);
                                        //loadedMesh.vertices.push_back(v.y);
                                        //loadedMesh.vertices.push_back(v.z);
                                        //if(i == 3)return;
                                    }
                                }
                                break;
                            case TINYGLTF_COMPONENT_TYPE_DOUBLE: {
                                std::cout << "Type is DOUBLE\n";
                                switch (attribAccessor.type) {
                                case TINYGLTF_TYPE_VEC3: {
                                    v3dArray positions(
                                        arrayAdapter<v3d>(dataPtr, count, byte_stride));
                                    for (size_t i{ 0 }; i < positions.size(); ++i) {
                                        const auto v = positions[i];
                                        std::cout << "positions[" << i << "]: (" << v.x
                                            << ", " << v.y << ", " << v.z << ")\n";

                                        //loadedMesh.vertices.push_back(v.x * scale);
                                        //loadedMesh.vertices.push_back(v.y * scale);
                                        //loadedMesh.vertices.push_back(v.z * scale);
                                    }
                                } break;
                                default:
                                    // TODO Handle error
                                    break;
                                }
                                break;
                            default:
                                break;
                            }
                            } break;
                            }
                        }
                    }
                }
                }
            }
        }
    }
}

void Scene::Clear()
{
    m_primitives.clear();
}

const std::vector<Primitive>& Scene::GetPrimitives() const
{
    return m_primitives;
}
