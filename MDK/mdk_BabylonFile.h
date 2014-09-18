#ifndef MDK_BABYLONFILE_H_INCLUDED
#define MDK_BABYLONFILE_H_INCLUDED

#include <AppConfig.h>
#include <modules/juce_core/juce_core.h>

namespace mdk
{


class BabylonFile
{
public:
    template<typename T>
    class Buffer : public ReferenceCountedObject
    {
    public:
        Array<T> data;
        typedef ReferenceCountedObjectPtr<Buffer> Ptr;
    };

    typedef Buffer<float> BufferF;
    typedef Buffer<int> BufferI32;

    class Var;

    class Mesh
    {
    public:
        BufferI32::Ptr indices;
        BufferF::Ptr positions;
        BufferF::Ptr normals;
        BufferF::Ptr uvs;
        BufferF::Ptr uvs2;
        BufferF::Ptr colors;

        float pivotMatrix[16];
        float position[3];
        float rotation[3];
        float scaling[3];

        String name;
        String id;
        String parentId;
    };

    class Texture
    {
        String name;
        int coordIndex;
        float uOffset;
        float vOffset;
        float uScale;
        float vScale;
        float uAng;
        float vAng;
        float wAng;
        float uWrap;
        float vWrap;
    };

    class Material
    {
    public:
        String name;
        String id;

        float ambient[3];
        float diffuse[3];
        float specular[3];
        float emissive[3];
        float specularPower;
        float alpha;
        bool backFaceCulling;
        Texture textureAmbient;
        Texture textureDiffuse;
        Texture textureSpecular;
        Texture textureEmissive;
    };

protected:
    bool importMeshes (const var& document);
    void importMesh (const var& _);

    bool importMaterials (const var& document);
    void importMaterial (const var& _);
    void importTexture (const var& _);

public:
    OwnedArray<Mesh> meshes;
    OwnedArray<Material> materials;

    BabylonFile();
    ~BabylonFile();

    bool read (InputStream* stream);
};

} // namespace

#endif	// MDK_BABYLONFILE_H_INCLUDED
