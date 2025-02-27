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

Model LoadGLTF(const char* path, const mat4& t)
{
    //https://github.com/syoyo/tinygltf/blob/master/examples/raytrace/gltf-loader.cc
    // TODO(syoyo): Texture
    // TODO(syoyo): Material

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        std::cout << "glTF parse warning: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "glTF parse error: " << err << std::endl;
    }
    if (!ret) {
        std::cerr << "Failed to load glTF: " << path << std::endl;
        return {};
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

    // Create model
    Model object;

    // Iterate through all the meshes in the glTF file
    for (const auto& gltfMesh : model.meshes)
    {
        std::cout << "Current mesh has " << gltfMesh.primitives.size()
            << " primitives:\n";

        // Create new mesh
        object.meshes.push_back({});
        Mesh& mesh = object.meshes.back();

        // For each primitive
        for (const auto& meshPrimitive : gltfMesh.primitives)
        {
            // Boolean used to check if we have converted the vertex buffer format
            bool convertedToTriangleList = false;

            // ----------------------
            // Load indices
            // ----------------------
            // This permit to get a type agnostic way of reading the index buffer
            std::unique_ptr<intArrayBase> indicesArrayPtr = nullptr;
            {
                const auto& indicesAccessor = model.accessors[meshPrimitive.indices];
                const auto& bufferView = model.bufferViews[indicesAccessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const auto dataAddress = buffer.data.data() + bufferView.byteOffset +
                    indicesAccessor.byteOffset;
                const auto byteStride = indicesAccessor.ByteStride(bufferView);
                const auto count = indicesAccessor.count;

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
            }

            // Convert indices
            const auto& indices = *indicesArrayPtr;
            if (indicesArrayPtr)
            {
                std::cout << "indices: ";
                for (size_t i(0); i < indicesArrayPtr->size(); ++i)
                {
                    std::cout << indices[i] << " ";
                    //loadedMesh.faces.push_back(indices[i]);
                }
                std::cout << '\n';
            }

            // ----------------------
            // Intepret vertices with indices to load mesh
            // ----------------------
            switch (meshPrimitive.mode)
            {
            case TINYGLTF_MODE_TRIANGLES:  // this is the simpliest case to handle
            {
                for (const auto& attribute : meshPrimitive.attributes)
                {
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

                    // -- Load attributes -- //
                    if (attribute.first == "POSITION")
                    {
                        std::vector<float> vertices;
                        switch (attribAccessor.type)
                        {
                        case TINYGLTF_TYPE_VEC3:
                        {
                            switch (attribAccessor.componentType)
                            {
                            case TINYGLTF_COMPONENT_TYPE_FLOAT:
                            {
                                v3fArray positions(arrayAdapter<v3f>(dataPtr, count, byte_stride));

                                for (size_t i{ 0 }; i < positions.size(); ++i)
                                {
                                    const auto va = positions[i];
                                    float3 v = make_float3(va.x, va.y, va.z);
                                    v = t.TransformPoint(v);

                                    vertices.push_back(v.x);
                                    vertices.push_back(v.y);
                                    vertices.push_back(v.z);
                                }
                            }
                            }
                        }
                        }
                        // Fill by adding a face a time
                        for (size_t i = 0; i < indices.size(); i += 3)
                        {
                            mesh.faces.push_back({});
                            auto& face = mesh.faces.back();

                            for (size_t j = 0; j < 3; j++)
                            {
                                // * 3 since attribs are 3 comps each
                                face[j].x = vertices[indices[i + j] * 3 + 0];
                                face[j].y = vertices[indices[i + j] * 3 + 1];
                                face[j].z = vertices[indices[i + j] * 3 + 2];
                            }
                        }
                    }
                    if (attribute.first == "NORMAL") 
                    {
                        std::vector<float> normalsVec;
                        switch (attribAccessor.type) 
                        {
                        case TINYGLTF_TYPE_VEC3: 
                        {
                            switch (attribAccessor.componentType) 
                            {
                            case TINYGLTF_COMPONENT_TYPE_FLOAT: 
                            {
                                std::cout << "Normal is FLOAT\n";
                                v3fArray normals(
                                    arrayAdapter<v3f>(dataPtr, count, byte_stride));

                                for (size_t i{ 0 }; i < normals.size(); i++) {
                                    const auto va = normals[i];
                                    float3 v = make_float3(va.x, va.y, va.z);
                                    v = t.TransformVector(v);

                                    // Put them in the array in the correct order
                                    normalsVec.push_back(v.x);
                                    normalsVec.push_back(v.y);
                                    normalsVec.push_back(v.z);
                                }
                            } break;
                            default:
                                std::cerr << "Unhandeled componant type for normal\n";
                            }
                        } break;
                        default:
                            std::cerr << "Unhandeled vector type for normal\n";
                        }
                    
                        // Fill mesh faces
                        for (size_t i = 0; i < indices.size(); i += 3)
                        {
                            mesh.normals.push_back({});
                            auto& normal = mesh.normals.back();

                            // * 3 since attribs are 3 comps each
                            normal.x = normalsVec[indices[i] * 3 + 0];
                            normal.y = normalsVec[indices[i] * 3 + 1];
                            normal.z = normalsVec[indices[i] * 3 + 2];
                        }
                    }

                    /*
                        // Face varying comment on the normals is also true for the UVs
                        if (attribute.first == "TEXCOORD_0") {
                            std::cout << "Found texture coordinates\n";

                            switch (attribAccessor.type) {
                            case TINYGLTF_TYPE_VEC2: {
                                std::cout << "TEXTCOORD is VEC2\n";
                                switch (attribAccessor.componentType) {
                                case TINYGLTF_COMPONENT_TYPE_FLOAT: {
                                    std::cout << "TEXTCOORD is FLOAT\n";
                                    v2fArray uvs(
                                        arrayAdapter<v2f>(dataPtr, count, byte_stride));

                                    for (size_t i{ 0 }; i < indices.size() / 3; ++i) {
                                        // get the i'th triange's indexes
                                        auto f0 = indices[3 * i + 0];
                                        auto f1 = indices[3 * i + 1];
                                        auto f2 = indices[3 * i + 2];

                                        // get the texture coordinates for each triangle's
                                        // vertices
                                        v2f uv0, uv1, uv2;
                                        uv0 = uvs[f0];
                                        uv1 = uvs[f1];
                                        uv2 = uvs[f2];

                                        // push them in order into the mesh data
                                        loadedMesh.facevarying_uvs.push_back(uv0.x);
                                        loadedMesh.facevarying_uvs.push_back(uv0.y);

                                        loadedMesh.facevarying_uvs.push_back(uv1.x);
                                        loadedMesh.facevarying_uvs.push_back(uv1.y);

                                        loadedMesh.facevarying_uvs.push_back(uv2.x);
                                        loadedMesh.facevarying_uvs.push_back(uv2.y);
                                    }

                                } break;
                                case TINYGLTF_COMPONENT_TYPE_DOUBLE: {
                                    std::cout << "TEXTCOORD is DOUBLE\n";
                                    v2dArray uvs(
                                        arrayAdapter<v2d>(dataPtr, count, byte_stride));

                                    for (size_t i{ 0 }; i < indices.size() / 3; ++i) {
                                        // get the i'th triange's indexes
                                        auto f0 = indices[3 * i + 0];
                                        auto f1 = indices[3 * i + 1];
                                        auto f2 = indices[3 * i + 2];

                                        v2d uv0, uv1, uv2;
                                        uv0 = uvs[f0];
                                        uv1 = uvs[f1];
                                        uv2 = uvs[f2];

                                        loadedMesh.facevarying_uvs.push_back(uv0.x);
                                        loadedMesh.facevarying_uvs.push_back(uv0.y);

                                        loadedMesh.facevarying_uvs.push_back(uv1.x);
                                        loadedMesh.facevarying_uvs.push_back(uv1.y);

                                        loadedMesh.facevarying_uvs.push_back(uv2.x);
                                        loadedMesh.facevarying_uvs.push_back(uv2.y);
                                    }
                                } break;
                                default:
                                    std::cerr << "unrecognized vector type for UV";
                                }
                            } break;
                            default:
                                std::cerr << "unreconized componant type for UV";
                            }
                        }
                    }
                    */
                    
                    // -- Load materials -- //
                    tinygltf::Material &mat = model.materials[meshPrimitive.material];

                    // Color
                    // Convert color to unsigned int
                    auto color = mat.pbrMetallicRoughness.baseColorFactor;
                    std::vector<uint> comp(4);
                    std::transform(std::begin(color), std::end(color), std::begin(comp), [](double val)
                    {return static_cast<uint>(val * 0xff); });

                    mesh.mat.color = *static_cast<Pixel*>(comp.data());
                }
            }
            break;

            default:
                std::cerr << "primitive mode not implemented";
                break;
            }

            std::cout << "Loaded vertices: " << mesh.faces.size() * 9 << '\n';
        }
    }

    /*
    // Iterate through all texture declaration in glTF file
    for (const auto& gltfTexture : model.textures) {
        std::cout << "Found texture!";
        Texture loadedTexture;
        const auto& image = model.images[gltfTexture.source];
        loadedTexture.components = image.component;
        loadedTexture.width = image.width;
        loadedTexture.height = image.height;

        const auto size =
            image.component * image.width * image.height * sizeof(unsigned char);
        loadedTexture.image = new unsigned char[size];
        memcpy(loadedTexture.image, image.image.data(), size);
        textures->push_back(loadedTexture);
    }
    */

    return object;
}
