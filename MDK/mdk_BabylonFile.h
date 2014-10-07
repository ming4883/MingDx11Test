#ifndef MDK_BABYLONFILE_H_INCLUDED
#define MDK_BABYLONFILE_H_INCLUDED

#include <AppConfig.h>
#include <modules/juce_core/juce_core.h>

#include "mdk_Math.h"

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

        inline int sizeInBytes() const
        {
            return data.size() * sizeof (T);
        }

        inline int size() const
        {
            return data.size();
        }

        inline T* getPtr()
        {
            return data.begin();
        }

        inline const T* getPtr() const
        {
            return data.begin();
        }
    };

    typedef Buffer<float> BufferF;
    typedef Buffer<int> BufferI32;

    class Var;

    class Animation
    {
    public:
        String name;
        int dataType;
        int framePerSecond;
        BufferI32 keyTimes;
        BufferF keyValues;
    };

    class SubMesh
    {
    public:
    };

    class Mesh
    {
    public:
        BufferI32::Ptr indices;
        BufferF::Ptr positions;
        BufferF::Ptr normals;
        BufferF::Ptr uvs;
        BufferF::Ptr uvs2;
        BufferF::Ptr colors;

        Mat44f pivotMatrix;
        Transform3f local;
        Transform3f world; //!< derived from local and parent's local

        String name;
        String id;
        String parentId;
        String materialId;

        juce::OwnedArray<Animation> animations;
    };

    class Texture
    {
    public:
        String name;
        int coordIndex;
        float uOffset;
        float vOffset;
        float uScale;
        float vScale;
        float uAng;
        float vAng;
        float wAng;
        bool uWrap;
        bool vWrap;
    };

    class Material
    {
    public:
        String name;
        String id;

        Vec3f ambient;
        Vec3f diffuse;
        Vec3f specular;
        Vec3f emissive;
        float specularPower;
        float alpha;
        bool backFaceCulling;
        ScopedPointer<Texture> textureAmbient;
        ScopedPointer<Texture> textureDiffuse;
        ScopedPointer<Texture> textureSpecular;
        ScopedPointer<Texture> textureEmissive;
    };


    class Adapter
    {
    public:
        virtual ~Adapter() {}

        virtual void adopt (Mesh* mesh, Material* material) = 0;
    };

protected:
    bool importMeshes (const var& document);
    void importMesh (const var& _);

    bool importMaterials (const var& document);
    void importMaterial (const var& _);
    Texture* importTexture (const var& _);
    Animation* importAnimation (const var& _);

public:
    OwnedArray<Mesh> meshes;
    OwnedArray<Material> materials;

    BabylonFile();
    ~BabylonFile();

    bool read (InputStream* stream);
    void adopt (Adapter* adapter);

protected:
    Mesh* getMesh (String id);
    Material* getMaterial (String id);
};

} // namespace

#endif // MDK_BABYLONFILE_H_INCLUDED
