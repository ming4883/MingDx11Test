/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"

using namespace mdk;

//==============================================================================
class Example02 : public D3D11Demo
{
public:
    Example02()
    {
    }
    
    ~Example02()
    {
    }

    class MyMaterial : public D3D11Material
    {
    public:
        Hold<ID3D11VertexShader> vs;
        Hold<ID3D11PixelShader> ps;
        Hold<ID3D11Buffer> cbSceneData;
        Hold<ID3D11Buffer> cbObjectData;
        D3D11ShaderResourcePool shaderResPool;
        D3D11ShaderResources vsResources;
        D3D11ShaderResources psResources;

        MyMaterial()
            : shaderResPool (16u, 16u)
            , vsResources (shaderResPool)
            , psResources (shaderResPool)
        {
        }

        void prepare (ID3D11DeviceContext* context, D3D11Scene* scene, D3D11DrawUnit* unit) override
        {
            ID3D11Buffer* cb[] = {
                cbSceneData,
                cbObjectData,
            };

            D3D11Scene::CBObjectData objData;
            objData.objAnimationTime.x = 0;
            Mat::mul (objData.objWorldMatrix, unit->worldMatrix, unit->pivotMatrix);
            objData.objNormalMatrix = objData.objWorldMatrix;
            Mat::mul (objData.objWorldViewProjMatrix, scene->sceneData.scnViewProjMatrix, objData.objWorldMatrix);

            D3D11Context::updateBuffer (context, cbObjectData, objData);
            
            context->VSSetShader (vs, nullptr, 0);
            context->VSSetConstantBuffers (0, numElementsInArray (cb), cb);

            context->PSSetShader (ps, nullptr, 0);
            context->PSSetConstantBuffers (0, numElementsInArray (cb), cb);

            D3D11ShaderResource* itr;

            itr = vsResources.list;

            while (itr)
            {
                context->VSSetShaderResources (itr->slot, 1, itr->objectSRView);
                itr = itr->nextListItem;
            }

            itr = psResources.list;

            while (itr)
            {
                context->PSSetShaderResources (itr->slot, 1, itr->objectSRView);
                itr = itr->nextListItem;
            }
        }
    };

    class MyBabylonFileAdaptor : public D3D11BabylonFileAdaptor
    {
    public:
        Hold<ID3DBlob> vsByteCode;
        Hold<ID3DBlob> psByteCode;

        Hold<ID3D11VertexShader> vs;
        Hold<ID3D11PixelShader> ps;

        MyBabylonFileAdaptor (D3D11Context& d3d11, D3D11Scene& scene)
            : D3D11BabylonFileAdaptor (d3d11, scene)
        {
            vsByteCode.set (d3d11.loadShaderFromAppData ("test.vso"));
            psByteCode.set (d3d11.loadShaderFromAppData ("test.pso"));

            vs.set (d3d11.createVertexShader (vsByteCode, nullptr));
            ps.set (d3d11.createPixelShader (psByteCode, nullptr));
        }

        D3D11Material* adopt (BabylonFile::Material* material)
        {
            MyMaterial* mtl = new MyMaterial;
            mtl->vs.set (vs, true);
            mtl->ps.set (ps, true);
            mtl->cbSceneData.set (scene.cbSceneData, true);
            mtl->cbObjectData.set (d3d11.createConstantBuffer<D3D11Scene::CBObjectData>());

            if (material && material->textureDiffuse)
            {
                const char* file = material->textureDiffuse->name.toRawUTF8();

                Hold<ID3D11Texture2D> d3dtex;
                Hold<ID3D11ShaderResourceView> srview;

                d3dtex.set (d3d11.createTexture2DFromAppData (file));

                srview.set (d3d11.createShaderResourceView (d3dtex));

                mtl->psResources.add (d3dtex.drop(), srview.drop(), 0);
            }

            return mtl;
        }

        ID3D11InputLayout* createInputLayout (const Array<D3D11_INPUT_ELEMENT_DESC>& inputElements)
        {
            return d3d11.createInputLayout (inputElements, vsByteCode);
        }
    };

    bool demoStartup()
    {
        appDataAddDir (m_dir_of_cpp().getSiblingFile ("Media"));

        scene_ = new D3D11Scene (d3d11);

        ScopedPointer<InputStream> stream = appDataGet ("Rabbit.babylon");

        if (m_isnull (stream))
            return false;

        BabylonFile file;
        file.read (stream);

        MyBabylonFileAdaptor adapter (d3d11, *scene_);
        
        file.adopt (&adapter);

        return true;
    }

    void demoShutdown()
    {
        cbFrameData_.set (nullptr);
        scene_ = nullptr;
    }

    void demoUpdate ()
    {
        D3D11_VIEWPORT vp = getViewport (0.0f, 0.0f, 1.0f, 1.0f);

        // update
        float sinTime, cosTime;
        Scalar::calcSinCos<float> (sinTime, cosTime, timeGetAccumMS<float>() / 4096.0f);
        float x = 200 * sinTime;
        float z = 200 * cosTime;

        Transform::fromLookAt (cam_.transform, Vec3f (x, 100, z), Vec3f (0, 0, 0), Vec3f (0, 1, 0));
        cam_.updateD3D (getAspect());
        scene_->update (d3d11, cam_, timeGetDeltaMS<float>() / 1000.0f);

        // render
        d3d11.contextIM->ClearRenderTargetView (d3d11.backBufRTView, (const float*)Vec4f (0.25f, 0.25f, 0.25f, 1.0f));
        d3d11.contextIM->ClearDepthStencilView (d3d11.depthBufDSView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        
        d3d11.contextIM->OMSetRenderTargets (1, d3d11.backBufRTView, d3d11.depthBufDSView);
        d3d11.contextIM->OMSetDepthStencilState (d3d11.depthTestOnWriteOn, 0);
        
        d3d11.contextIM->RSSetViewports (1, &vp);
        d3d11.contextIM->RSSetState (d3d11.rastCullBack);
        
        scene_->drawAll (d3d11.contextIM);

        d3d11.swapchain->Present (0u, 0u);
    }

    Hold<ID3D11Buffer> cbFrameData_;
    ScopedPointer<D3D11Scene> scene_;
    Camera cam_;
};

//==============================================================================
class Example02Application  : public DemoApplication<Example02>
{
public:
    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (Example02Application)
