#ifndef MDK_D3D11SCENE_H_INCLUDED
#define MDK_D3D11SCENE_H_INCLUDED

#include "mdk_D3D11.h"
#include "mdk_BabylonFile.h"

namespace mdk
{

class D3D11DrawUnit
{
public:
    Hold<ID3D11Buffer> indexBuffer;
    Hold<ID3D11Buffer> positionBuffer;
    Hold<ID3D11Buffer> normalBuffer;
    Hold<ID3D11Buffer> uvBuffer;
    Hold<ID3D11Buffer> uv2Buffer;
    Hold<ID3D11Buffer> colorBuffer;
    Hold<ID3D11InputLayout> inputLayout;

    Hold<ID3D11VertexShader> vertexShader;
    Hold<ID3D11PixelShader> pixelShader;

    Hold<ID3D11BlendState> blendState;
    Hold<ID3D11DepthStencilState> depthState;

    Mat44f pivotMatrix;
    Mat44f worldMatrix;
};

class D3D11Scene
{
public:
    OwnedArray<D3D11DrawUnit> drawUnits;

    D3D11DrawUnit* add();
};

class D3D11BabylonFileAdaptor : public BabylonFile::Adapter
{
    D3D11BabylonFileAdaptor (D3D11BabylonFileAdaptor&);
    D3D11BabylonFileAdaptor& operator = (D3D11BabylonFileAdaptor&);

public:
    D3D11Context& d3d11;
    D3D11Scene& scene;

    D3D11BabylonFileAdaptor (D3D11Context& d3d11, D3D11Scene& scene);

    void adopt (BabylonFile::Mesh* mesh, BabylonFile::Material* material, int drawStart, int drawCnt) override;
};

} // namespace

#endif	// MDK_D3D11SCENE_H_INCLUDED
