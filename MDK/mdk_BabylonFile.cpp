#include "mdk_BabylonFile.h"

#define decl_id(nameOfId) static const Identifier id_##nameOfId (#nameOfId);

namespace mdk
{

class BabylonFile::Var
{
public:
    static inline bool isNull (const var& _)
    {
        return _.isUndefined() || _.isVoid();
    }

    template<typename T>
    static inline typename Buffer<T>::Ptr toBuffer (const var& arr)
    {
        typename Buffer<T>::Ptr ret;
        if (isNull (arr) || !arr.isArray())
            return ret;

        ret = new Buffer<T>;
        for (int i = 0; i < arr.size(); ++i)
            ret->data.add ((T)arr[i]);

        return ret;
    }

    template<typename T>
    static inline typename Buffer<T>::Ptr toBuffer (const var& src, const Identifier& id)
    {
        return toBuffer<T> (src.getProperty (id, var::null));
    }

    template<typename T, int N>
    static inline bool toVector (T* dst, const var& src, T* defaultVals)
    {
        (void) sizeof (0[dst]); // This line should cause an error if you pass an object with a user-defined subscript operator

        if (isNull (src) || src.size() < N)
        {
            memcpy (dst, defaultVals, sizeof (T) * N);
            return false;
        }
        
        for (int i = 0; i < N; ++i)
            dst[i] = (T)src[i];

        return true;
    }

    template<typename T>
    static inline bool toVector (Vec3<T>& dst, const var& src, const Identifier& id, T* defaultVals)
    {
        return toVector<T, 3> ((T*)dst, src.getProperty (id, var::null), defaultVals);
    }

    template<typename T>
    static inline bool toVector (Vec4<T>& dst, const var& src, const Identifier& id, T* defaultVals)
    {
        return toVector<T, 4> ((T*)dst, src.getProperty (id, var::null), defaultVals);
    }

    template<typename T>
    static inline bool toVector (Mat44<T>& dst, const var& src, const Identifier& id, T* defaultVals)
    {
        return toVector<T, 16> ((T*)dst.m, src.getProperty (id, var::null), defaultVals);
    }

    template<typename T>
    static inline bool toScalar (T& dst, const var& src, T defaultVal)
    {
        if (isNull (src))
        {
            dst = defaultVal;
            return false;
        }
        
        dst = (T)src;
        return true;
    }

    template<typename T>
    static inline bool toScalar (T& dst, const var& src, const Identifier& id, T defaultVal)
    {
        return toScalar (dst, src.getProperty (id, var::null), defaultVal);
    }
};

BabylonFile::BabylonFile()
{
}

BabylonFile::~BabylonFile()
{
}

bool BabylonFile::read (InputStream* stream)
{
    String content = stream->readEntireStreamAsString();
    var document = JSON::fromString (content);

    if (Var::isNull (document))
        return false;
    
    if (!importMeshes (document))
        return false;

    if (!importMaterials (document))
        return false;
    
    return true;
}

bool BabylonFile::importMeshes (const var& document)
{
    static const Identifier id_meshes ("meshes");

    var arr = document.getProperty (id_meshes, var::null);

    if (Var::isNull (arr) || !arr.isArray())
        return false;

    for (int i = 0; i < arr.size(); ++i)
    {
        importMesh (arr[i]);
    }

    return true;
}

void BabylonFile::importMesh (const var& _)
{
    decl_id (name);
    decl_id (id);
    decl_id (parentId);
    decl_id (materialId);

    decl_id (position);
    decl_id (rotation);
    decl_id (rotationQuaternion);
    decl_id (scaling);
    decl_id (pivotMatrix);

    decl_id (indices);
    decl_id (positions);
    decl_id (normals);
    decl_id (uvs);
    decl_id (uvs2);
    decl_id (colors);
    
    ScopedPointer<Mesh> mesh = new Mesh;

    mesh->name = _.getProperty (id_name, "").toString();
    mesh->id = _.getProperty (id_id, "").toString();
    mesh->parentId = _.getProperty (id_parentId, "").toString();
    mesh->materialId = _.getProperty (id_materialId, "").toString();

    {
        float def[3] = {0.0f, 0.0f, 0.0f};
        Var::toVector (mesh->local.position, _, id_position, def);
    }

    {
        float def[3] = {0.0f, 0.0f, 0.0f};
        Vec3f rotation;
        if (Var::toVector (rotation, _, id_rotation, def))
            mesh->local.rotation = Quat::fromRotation (rotation);
    }

    {
        float def[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        Var::toVector (mesh->local.rotation, _, id_rotationQuaternion, def);
    }

    {
        float def[3] = {1.0f, 1.0f, 1.0f};
        Var::toVector (mesh->local.scaling, _, id_scaling, def);
    }
    
    {
        float def[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
        };
        Var::toVector (mesh->pivotMatrix, _, id_pivotMatrix, def);
    }

    mesh->indices   = Var::toBuffer<int> (_, id_indices);
    mesh->positions = Var::toBuffer<float> (_, id_positions);
    mesh->normals   = Var::toBuffer<float> (_, id_normals);
    mesh->uvs       = Var::toBuffer<float> (_, id_uvs);
    mesh->uvs2      = Var::toBuffer<float> (_, id_uvs2);
    mesh->colors    = Var::toBuffer<float> (_, id_colors);

    // compute transform
    Mesh* parent = getMesh (mesh->parentId);

    if (parent)
    {
        Transform::derive (mesh->world, parent->world, mesh->local);
    }
    else
    {
        mesh->world = mesh->local;
    }

    meshes.add (mesh.release());
}

bool BabylonFile::importMaterials (const var& document)
{
    static const Identifier id_materials ("materials");

    var arr = document.getProperty (id_materials, var::null);

    if (Var::isNull (arr) || !arr.isArray())
        return false;

    for (int i = 0; i < arr.size(); ++i)
    {
        importMaterial (arr[i]);
    }

    return true;
}

void BabylonFile::importMaterial (const var& _)
{
    decl_id (name);
    decl_id (id);

    decl_id (ambient);
    decl_id (diffuse);
    decl_id (specular);
    decl_id (emissive);
    decl_id (specularPower);
    decl_id (alpha);
    decl_id (backFaceCulling);
    decl_id (ambientTexture);
    decl_id (diffuseTexture);
    decl_id (specularTexture);
    decl_id (emissiveTexture);

    ScopedPointer<Material> mtl = new Material;
    {
        float def[3] = {1.0f, 1.0f, 1.0f};
        Var::toVector (mtl->ambient, _, id_ambient, def);
    }
    {
        float def[3] = {1.0f, 1.0f, 1.0f};
        Var::toVector (mtl->diffuse, _, id_diffuse, def);
    }
    {
        float def[3] = {1.0f, 1.0f, 1.0f};
        Var::toVector (mtl->specular, _, id_specular, def);
    }
    {
        float def[3] = {1.0f, 1.0f, 1.0f};
        Var::toVector (mtl->emissive, _, id_emissive, def);
    }

    Var::toScalar (mtl->specularPower, _, id_specularPower, 1.0f);
    Var::toScalar (mtl->alpha, _, id_alpha, 1.0f);
    Var::toScalar (mtl->backFaceCulling, _, id_backFaceCulling, true);

    mtl->textureAmbient  = importTexture (_.getProperty (id_ambientTexture, var::null));
    mtl->textureDiffuse  = importTexture (_.getProperty (id_diffuseTexture, var::null));
    mtl->textureSpecular = importTexture (_.getProperty (id_specularTexture, var::null));
    mtl->textureEmissive = importTexture (_.getProperty (id_emissiveTexture, var::null));

    materials.add (mtl.release());
}

BabylonFile::Texture* BabylonFile::importTexture (const var& _)
{
    decl_id (name);
    decl_id (uOffset);
    decl_id (vOffset);
    decl_id (uScale);
    decl_id (vScale);
    decl_id (uAng);
    decl_id (vAng);
    decl_id (wAng);
    decl_id (wrapU);
    decl_id (wrapV);
    decl_id (coordinatesIndex);

    if (Var::isNull (_))
        return nullptr;

    ScopedPointer<Texture> tex = new Texture;

    tex->name = _.getProperty (id_name, var::null);

    if (tex->name.isEmpty())
        return nullptr;

    Var::toScalar (tex->uOffset, _, id_uOffset, 0.0f);
    Var::toScalar (tex->vOffset, _, id_vOffset, 0.0f);
    Var::toScalar (tex->uScale, _, id_uScale, 1.0f);
    Var::toScalar (tex->vScale, _, id_vScale, 1.0f);
    Var::toScalar (tex->uAng, _, id_uAng, 0.0f);
    Var::toScalar (tex->vAng, _, id_vAng, 0.0f);
    Var::toScalar (tex->wAng, _, id_wAng, 0.0f);
    Var::toScalar (tex->uWrap, _, id_wrapU, true);
    Var::toScalar (tex->vWrap, _, id_wrapV, true);
    Var::toScalar (tex->coordIndex, _, id_coordinatesIndex, 0);

    return tex.release();
}

void BabylonFile::adopt (Adapter* adapter)
{
    Mesh** meshBeg = meshes.begin();
    Mesh** meshEnd = meshes.end();

    for (Mesh** meshItr = meshBeg; meshItr != meshEnd; ++meshItr)
    {
        Mesh* mesh = *meshItr;
        Material* mtl = getMaterial (mesh->materialId);

        int drawStart = 0;
        int drawCnt = 0;

        if (mesh->indices != nullptr)
            drawCnt = mesh->indices->data.size();
        else
            drawCnt = mesh->positions->data.size();

        adapter->adopt (mesh, mtl, drawStart, drawCnt);
    }
}

BabylonFile::Mesh* BabylonFile::getMesh (String id)
{
    Mesh** beg = meshes.begin();
    Mesh** end = meshes.end();

    for (Mesh** itr = beg; itr != end; ++itr)
    {
        Mesh* cur = *itr;
        if (cur->id == id)
            return cur;
    }

    return nullptr;
}

BabylonFile::Material* BabylonFile::getMaterial (String id)
{
    Material** beg = materials.begin();
    Material** end = materials.end();

    for (Material** itr = beg; itr != end; ++itr)
    {
        Material* cur = *itr;
        if (cur->id == id)
            return cur;
    }

    return nullptr;
}

} // namespace
