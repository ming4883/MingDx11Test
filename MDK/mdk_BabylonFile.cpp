#include "mdk_BabylonFile.h"

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
    static typename Buffer<T>::Ptr toBuffer (const var& arr)
    {
        typename Buffer<T>::Ptr ret;
        if (isNull (arr) || !arr.isArray())
            return ret;

        ret = new Buffer<T>;
        for (int i = 0; i < arr.size(); ++i)
            ret->data.add ((T)arr[i]);

        return ret;
    }

    template<typename T, int N>
    static bool toArray (T (&dst)[N], const var& src)
    {
        (void) sizeof (0[dst]); // This line should cause an error if you pass an object with a user-defined subscript operator

        if (isNull (src) || src.size() < N)
            return false;

        for (int i = 0; i < N; ++i)
            dst[i] = (T)src[i];

        return true;
    }

    template<typename T>
    static bool toScalar (T& dst, const var& src)
    {
        if (isNull (src))
            return false;

        dst = (T)src;
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
    static const Identifier id_name ("name");
    static const Identifier id_id ("id");
    static const Identifier id_parentId ("parentId");

    static const Identifier id_position ("position");
    static const Identifier id_rotation ("rotation");
    static const Identifier id_scaling ("scaling");
    static const Identifier id_pivotMatrix ("pivotMatrix");

    static const Identifier id_indices ("indices");
    static const Identifier id_positions ("positions");
    static const Identifier id_normals ("normals");
    static const Identifier id_uvs ("uvs");
    static const Identifier id_uvs2 ("uvs2");
    static const Identifier id_colors ("colors");
    
    ScopedPointer<Mesh> mesh = new Mesh;

    mesh->name = _.getProperty (id_name, "").toString();
    mesh->id = _.getProperty (id_id, "").toString();
    mesh->parentId = _.getProperty (id_parentId, "").toString();

    Var::toArray (mesh->position, _.getProperty (id_position, var::null));
    Var::toArray (mesh->rotation, _.getProperty (id_rotation, var::null));
    Var::toArray (mesh->scaling, _.getProperty (id_scaling, var::null));
    Var::toArray (mesh->pivotMatrix, _.getProperty (id_pivotMatrix, var::null));

    mesh->indices   = Var::toBuffer<int> (_.getProperty (id_indices, var::null));
    mesh->positions = Var::toBuffer<float> (_.getProperty (id_positions, var::null));
    mesh->normals   = Var::toBuffer<float> (_.getProperty (id_normals, var::null));
    mesh->uvs       = Var::toBuffer<float> (_.getProperty (id_uvs, var::null));
    mesh->uvs2      = Var::toBuffer<float> (_.getProperty (id_uvs2, var::null));
    mesh->colors    = Var::toBuffer<float> (_.getProperty (id_colors, var::null));

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
    static const Identifier id_name ("name");
    static const Identifier id_id ("id");

    static const Identifier id_ambient ("ambient");
    static const Identifier id_diffuse ("diffuse");
    static const Identifier id_specular ("specular");
    static const Identifier id_emissive ("emissive");
    static const Identifier id_specularPower ("specularPower");
    static const Identifier id_alpha ("alpha");
    static const Identifier id_backFaceCulling ("backFaceCulling");
    static const Identifier id_ambientTexture ("ambientTexture");
    static const Identifier id_diffuseTexture ("diffuseTexture");
    static const Identifier id_specularTexture ("specularTexture");
    static const Identifier id_emissiveTexture ("emissiveTexture");

    ScopedPointer<Material> mtl = new Material;

    materials.add (mtl.release());
}

void BabylonFile::importTexture (const var& _)
{
}

} // namespace
