#include "mdk_D3D11Scene.h"


namespace mdk
{

D3D11DrawUnit* D3D11Scene::add()
{
    D3D11DrawUnit* unit = new D3D11DrawUnit;
    drawUnits.add (unit);
    return unit;
}

D3D11BabylonFileAdaptor::D3D11BabylonFileAdaptor (D3D11Context& d3d11, D3D11Scene& scene)
    : d3d11 (d3d11)
    , scene (scene)
{
}

void D3D11BabylonFileAdaptor::adopt (BabylonFile::Mesh* mesh, BabylonFile::Material* material, int drawStart, int drawCnt)
{
    if (nullptr == mesh->positions)
        return;

    D3D11DrawUnit* unit = scene.add();

    unit->pivotMatrix = mesh->pivotMatrix;
    Mat::fromTransform3 (unit->worldMatrix, mesh->world);

    // index
    if (mesh->indices)
    {
        auto buff = mesh->indices;
        unit->indexBuffer.set (d3d11.createIndexBuffer ((size_t)buff->size(), false, true, buff->getPtr()));
    }

    Array<D3D11_INPUT_ELEMENT_DESC> iDesc;

    // position
    {
        D3D11_INPUT_ELEMENT_DESC _;
        _.SemanticName = "POSITION";
        _.SemanticIndex = 0;
        _.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        _.InputSlot = 0;
        _.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        _.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        _.InstanceDataStepRate = 0;

        iDesc.add (_);

        auto buff = mesh->positions;
        unit->positionBuffer.set (d3d11.createVertexBuffer ((size_t)buff->size(), false, true, buff->getPtr()));
    }

    if (mesh->normals)
    {
        D3D11_INPUT_ELEMENT_DESC _;
        _.SemanticName = "NORMAL";
        _.SemanticIndex = 0;
        _.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        _.InputSlot = (UINT)iDesc.size();
        _.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        _.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        _.InstanceDataStepRate = 0;

        iDesc.add (_);

        auto buff = mesh->normals;
        unit->normalBuffer.set (d3d11.createVertexBuffer ((size_t)buff->size(), false, true, buff->getPtr()));
    }

    if (mesh->uvs)
    {
        D3D11_INPUT_ELEMENT_DESC _;
        _.SemanticName = "TEXCOORD";
        _.SemanticIndex = 0;
        _.Format = DXGI_FORMAT_R32G32_FLOAT;
        _.InputSlot = (UINT)iDesc.size();
        _.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        _.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        _.InstanceDataStepRate = 0;

        iDesc.add (_);

        auto buff = mesh->uvs;
        unit->uvBuffer.set (d3d11.createVertexBuffer ((size_t)buff->size(), false, true, buff->getPtr()));
    }

    if (mesh->uvs2)
    {
        D3D11_INPUT_ELEMENT_DESC _;
        _.SemanticName = "TEXCOORD";
        _.SemanticIndex = 1;
        _.Format = DXGI_FORMAT_R32G32_FLOAT;
        _.InputSlot = (UINT)iDesc.size();
        _.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        _.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        _.InstanceDataStepRate = 0;

        iDesc.add (_);

        auto buff = mesh->uvs2;
        unit->uv2Buffer.set (d3d11.createVertexBuffer ((size_t)buff->size(), false, true, buff->getPtr()));
    }

    if (mesh->colors)
    {
        D3D11_INPUT_ELEMENT_DESC _;
        _.SemanticName = "COLOR";
        _.SemanticIndex = 0;
        _.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        _.InputSlot = (UINT)iDesc.size();
        _.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        _.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        _.InstanceDataStepRate = 0;

        iDesc.add (_);

        auto buff = mesh->colors;
        unit->colorBuffer.set (d3d11.createVertexBuffer ((size_t)buff->size(), false, true, buff->getPtr()));
    }
}

} // namespace
