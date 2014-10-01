#ifndef MDK_D3D11SCENE_H_INCLUDED
#define MDK_D3D11SCENE_H_INCLUDED

#include "mdk_D3D11.h"
#include "mdk_BabylonFile.h"

namespace mdk
{

class D3D11DrawUnit;
class D3D11Scene;

class D3D11Material
{
public:
    virtual ~D3D11Material() {}
    virtual void prepare (ID3D11DeviceContext* context, D3D11Scene* scene, D3D11DrawUnit* unit) = 0;
};

class D3D11DrawUnit
{
public:
    typedef LinkedListPointer<D3D11DrawUnit> ListPtr;
    friend class ListPtr;
    friend class D3D11Scene;
    enum
    {
        cNumOfVBs = 5
    };

    Hold<ID3D11Buffer> indexBuffer;
    Hold<ID3D11Buffer> positionBuffer;
    Hold<ID3D11Buffer> normalBuffer;
    Hold<ID3D11Buffer> uvBuffer;
    Hold<ID3D11Buffer> uv2Buffer;
    Hold<ID3D11Buffer> colorBuffer;
    Hold<ID3D11InputLayout> inputLayout;

    ScopedPointer<D3D11Material> material;

    ID3D11Buffer* vbBindings[cNumOfVBs];
    UINT vbOffsets[cNumOfVBs];
    UINT vbStrides[cNumOfVBs];
    UINT vbCount;
    UINT vertexCount;
    UINT indexCount;
    D3D11_PRIMITIVE_TOPOLOGY topology;

    Mat44f pivotMatrix;
    Mat44f worldMatrix;

private:
    ListPtr nextListItem;
};

class D3D11Scene
{
public:
    cbuffer CBSceneData
    {
	    Vec4f scnAnimationTime;
	    Vec4f scnViewPos;
	    Mat44f scnViewProjMatrix;
    };

    cbuffer CBObjectData
    {
	    Vec4f objAnimationTime;
	    Mat44f objWorldMatrix;
	    Mat44f objNormalMatrix;
	    Mat44f objWorldViewProjMatrix;
    };

    typedef ObjectPool< ObjectPoolTraitsDefault<D3D11DrawUnit> > DrawUnitPool;

public:
    DrawUnitPool drawUnitPool;

    CBSceneData sceneData;
    Hold<ID3D11Buffer> cbSceneData;
    D3D11DrawUnit::ListPtr drawUnits;

public:
    D3D11Scene (D3D11Context& d3d11);
    ~D3D11Scene();

    void update (D3D11Context& d3d11, Demo::Camera& camera, float deltaTime);

    D3D11DrawUnit* add();

    void drawAll (ID3D11DeviceContext* context);
};


class D3D11BabylonFileAdaptor : public BabylonFile::Adapter
{
    D3D11BabylonFileAdaptor (D3D11BabylonFileAdaptor&);
    D3D11BabylonFileAdaptor& operator = (D3D11BabylonFileAdaptor&);

public:
    D3D11Context& d3d11;
    D3D11Scene& scene;

    D3D11BabylonFileAdaptor (D3D11Context& d3d11, D3D11Scene& scene);

    void adopt (BabylonFile::Mesh* mesh, BabylonFile::Material* material) override;

    virtual D3D11Material* adopt (BabylonFile::Material* material) = 0;
    virtual ID3D11InputLayout* createInputLayout (const Array<D3D11_INPUT_ELEMENT_DESC>& inputElements) = 0;
};

} // namespace

#endif	// MDK_D3D11SCENE_H_INCLUDED
