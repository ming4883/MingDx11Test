#include "mdk_D3D11Scene.h"


namespace mdk
{

/**/
D3D11SRBindings::D3D11SRBindings(D3D11SRBindingPool& pool)
    : pool_ (pool)
{

}

D3D11SRBindings::~D3D11SRBindings()
{
    D3D11SRBinding* itr = list;

    while (itr)
    {
        D3D11SRBinding* next = itr->nextListItem;

        m_del_with_pool (pool_, itr);

        itr = next;
    }
}

void D3D11SRBindings::add (ID3D11Resource* object, ID3D11ShaderResourceView* objectSRView, UINT slot)
{
    D3D11SRBinding* res = m_new_with_pool (pool_, D3D11SRBinding);
    res->object.set (object);
    res->objectSRView.set (objectSRView);
    res->slot = slot;

    list.append (res);
}

/**/
D3D11SampBindings::D3D11SampBindings(D3D11SampBindingPool& pool)
    : pool_ (pool)
{

}

D3D11SampBindings::~D3D11SampBindings()
{
    D3D11SampBinding* itr = list;

    while (itr)
    {
        D3D11SampBinding* next = itr->nextListItem;

        m_del_with_pool (pool_, itr);

        itr = next;
    }
}

void D3D11SampBindings::add (ID3D11SamplerState* sampler, UINT slot)
{
    D3D11SampBinding* res = m_new_with_pool (pool_, D3D11SampBinding);
    res->sampler.set (sampler);
    res->slot = slot;

    list.append (res);
}

/**/
D3D11SceneMaterial::D3D11SceneMaterial()
    : srBindingPool (16u, 16u)
    , srBindings (srBindingPool)
    , sampBindingPool (16u, 16u)
    , sampBindings (sampBindingPool)
{
}

void D3D11SceneMaterial::prepare (ID3D11DeviceContext* context, D3D11Scene* scene, D3D11DrawUnit* unit)
{
    D3D11Scene::CBObjectData objData;
    objData.objAnimationTime.x = 0;
    Mat::mul (objData.objWorldMatrix, unit->worldMatrix, unit->pivotMatrix);

    objData.objNormalMatrix = objData.objWorldMatrix;
    Mat::mul (objData.objWorldViewProjMatrix, scene->sceneData.scnViewProjMatrix, objData.objWorldMatrix);
            
    D3D11Context::updateBuffer (context, cbObjectData, objData);

    context->VSSetShader (vs, nullptr, 0);
    context->PSSetShader (ps, nullptr, 0);

    bindShaderConstants (context);
    bindShaderResources (context);
    bindShaderSamplers (context);
}

void D3D11SceneMaterial::bindShaderConstants (ID3D11DeviceContext* context)
{
    ID3D11Buffer* cb[] = {
        cbSceneData,
        cbObjectData,
    };

    context->VSSetConstantBuffers (0, numElementsInArray (cb), cb);
    context->PSSetConstantBuffers (0, numElementsInArray (cb), cb);
}

void D3D11SceneMaterial::bindShaderResources (ID3D11DeviceContext* context)
{
    D3D11SRBinding* itr = srBindings.list;

    while (itr)
    {
        context->VSSetShaderResources (itr->slot, 1, itr->objectSRView);
        context->PSSetShaderResources (itr->slot, 1, itr->objectSRView);
        itr = itr->nextListItem;
    }
}

void D3D11SceneMaterial::bindShaderSamplers (ID3D11DeviceContext* context)
{
    D3D11SampBinding* itr = sampBindings.list;

    while (itr)
    {
        context->VSSetSamplers (itr->slot, 1, itr->sampler);
        context->PSSetSamplers (itr->slot, 1, itr->sampler);
        itr = itr->nextListItem;
    }
}

/**/
D3D11Scene::D3D11Scene (D3D11Context& d3d11)
    : drawUnitPool (256, 1024)
{
    cbSceneData.set (d3d11.createConstantBuffer<CBSceneData>());
    zerostruct (sceneData);
}

D3D11Scene::~D3D11Scene()
{
    D3D11DrawUnit* itr = drawUnits;

    while (itr != nullptr)
    {
        D3D11DrawUnit* next = itr->nextListItem;
        m_del_with_pool (drawUnitPool, itr);
        itr = next;
    }
}

void D3D11Scene::update (D3D11Context& d3d11, Demo::Camera& camera, float deltaTime)
{   
    sceneData.scnAnimationTime.x += deltaTime;
    sceneData.scnViewPos = Vec4f (camera.transform.position, 1.0f);
    Mat::mul (sceneData.scnViewProjMatrix, camera.projectionMatrix, camera.viewMatrix);
    d3d11.updateBuffer (cbSceneData, sceneData);
}

D3D11DrawUnit* D3D11Scene::add()
{
    D3D11DrawUnit* unit = m_new_with_pool (drawUnitPool, D3D11DrawUnit);
    drawUnits.append (unit);
    return unit;
}

void D3D11Scene::drawAll (ID3D11DeviceContext* context)
{
    for (D3D11DrawUnit* itr = drawUnits; itr != nullptr; itr = itr->nextListItem)
    {
        D3D11DrawUnit* cur = itr;

        cur->material->prepare (context, this, cur);

        context->IASetVertexBuffers (0, cur->vbCount, cur->vbBindings, cur->vbStrides, cur->vbOffsets);
        context->IASetInputLayout (cur->inputLayout);
        context->IASetPrimitiveTopology (cur->topology);

        if (cur->indexBuffer != nullptr)
        {
            context->IASetIndexBuffer (cur->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
            context->DrawIndexed (cur->indexCount, 0, 0);
        }
        else
        {
            context->Draw (cur->vertexCount, 0);
        }
    }
}

D3D11BabylonFileAdaptor::D3D11BabylonFileAdaptor (D3D11Context& d3d11, D3D11Scene& scene)
    : d3d11 (d3d11)
    , scene (scene)
{
}

void D3D11BabylonFileAdaptor::adopt (BabylonFile::Mesh* mesh, BabylonFile::Material* material)
{
    if (nullptr == mesh->positions)
        return;

    D3D11DrawUnit* unit = scene.add();

    unit->material = adopt (material);
    unit->pivotMatrix = mesh->pivotMatrix;
    Mat::fromTransform3 (unit->worldMatrix, mesh->world);

    // topology information
    unit->topology =  D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    unit->indexCount = 0;
    unit->vertexCount = mesh->positions->size() / 3;

    // index
    if (mesh->indices)
    {
        auto buff = mesh->indices;
        unit->indexBuffer.set (d3d11.createIndexBuffer ((size_t)buff->sizeInBytes(), false, true, buff->getPtr()));
        unit->indexCount = buff->size();
    }

    Array<D3D11_INPUT_ELEMENT_DESC> iDesc;

    ID3D11Buffer** vbBinding = unit->vbBindings;
    UINT* vbOffset = unit->vbOffsets;
    UINT* vbStride = unit->vbStrides;

    zerostruct (unit->vbBindings);
    zerostruct (unit->vbOffsets);
    
    // positions
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
        unit->positionBuffer.set (d3d11.createVertexBuffer ((size_t)buff->sizeInBytes(), false, true, buff->getPtr()));
        *vbBinding++ = unit->positionBuffer;
        *vbOffset++ = 0;
        *vbStride++ = sizeof (float) * 3;
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
        unit->normalBuffer.set (d3d11.createVertexBuffer ((size_t)buff->sizeInBytes(), false, true, buff->getPtr()));
        *vbBinding++ = unit->normalBuffer;
        *vbOffset++ = 0;
        *vbStride++ = sizeof (float) * 3;
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
        float* buffPtr = buff->getPtr();
        for (int i = 1; i < buff->data.size(); i += 2)
        {
            buffPtr[i] = 1.0f - buffPtr[i];
        }

        unit->uvBuffer.set (d3d11.createVertexBuffer ((size_t)buff->sizeInBytes(), false, true, buff->getPtr()));
        *vbBinding++ = unit->uvBuffer;
        *vbOffset++ = 0;
        *vbStride++ = sizeof (float) * 2;
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
        unit->uv2Buffer.set (d3d11.createVertexBuffer ((size_t)buff->sizeInBytes(), false, true, buff->getPtr()));
        *vbBinding++ = unit->uv2Buffer;
        *vbOffset++ = 0;
        *vbStride++ = sizeof (float) * 2;
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
        unit->colorBuffer.set (d3d11.createVertexBuffer ((size_t)buff->sizeInBytes(), false, true, buff->getPtr()));
        *vbBinding++ = unit->colorBuffer;
        *vbOffset++ = 0;
        *vbStride++ = sizeof (float) * 4;
    }

    unit->vbCount = (UINT)iDesc.size();

    unit->inputLayout.set (createInputLayout (iDesc));
}

} // namespace
