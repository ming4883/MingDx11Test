/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"

using namespace mdk;

//==============================================================================
class TheDemo : public D3D11Demo
{
public:
    TheDemo()
    {
    }
    
    ~TheDemo()
    {
    }

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
            D3D11SceneMaterial* mtl = new D3D11SceneMaterial;
            mtl->vs.set (vs, true);
            mtl->ps.set (ps, true);
            mtl->cbSceneData.set (scene.cbSceneData, true);
            mtl->cbObjectData.set (d3d11.createConstantBuffer<D3D11Scene::CBObjectData>());

            if (material && material->textureDiffuse)
            {
                adoptTexture (mtl->srBindings, mtl->sampBindings, material->textureDiffuse, 0);
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
        setupMouseListener();

        appDataAddDir (m_dir_of_cpp().getSiblingFile ("Media"));

        scene_ = new D3D11Scene (d3d11);

        ScopedPointer<InputStream> stream = appDataGet ("Boxes.babylon");
        //ScopedPointer<InputStream> stream = appDataGet ("SponzaSimple.babylon");

        if (m_isnull (stream))
            return false;

        BabylonFile file;
        file.read (stream);

        MyBabylonFileAdaptor adapter (d3d11, *scene_);
        
        file.adopt (&adapter);

        Camera::Projection proj = cam_.projection.fetch();
        proj.zFar = 2048.0f;

        Transform3f tran = cam_.transform.fetch();
        tran.position = Vec3f (0, 100, 200);

        cam_.projection.commit (proj);
        cam_.transform.commit (tran);

        //Transform::fromLookAt (cam_.transform, Vec3f (x, 100, z), Vec3f (0, 50, 0), Vec3f (0, 1, 0));

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
#if 0
        float sinTime, cosTime;
        Scalar::calcSinCos<float> (sinTime, cosTime, timeGetAccumMS<float>() / 4096.0f);
        float x = 200 * sinTime;
        float z = 200 * cosTime;

        Transform::fromLookAt (cam_.transform, Vec3f (x, 100, z), Vec3f (0, 50, 0), Vec3f (0, 1, 0));
#endif
        ScopedSyncWrite<SyncWithAtomic> lock (fpsCamControl_->sync);

        cam_.updateD3D (getAspect());
        scene_->update (d3d11, cam_, timeGetDeltaMS<float>() / 1000.0f);

        // render
        d3d11.contextIM->ClearRenderTargetView (d3d11.backBufRTView, (const float*)Vec4f (0.25f, 0.25f, 0.25f, 1.0f));
        d3d11.contextIM->ClearDepthStencilView (d3d11.depthBufDSView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        
        d3d11.contextIM->OMSetRenderTargets (1, d3d11.backBufRTView, d3d11.depthBufDSView);
        d3d11.contextIM->OMSetDepthStencilState (d3d11.depthTestOnWriteOn, 0);
        
        d3d11.contextIM->RSSetViewports (1, &vp);
        d3d11.contextIM->RSSetState (d3d11.rastCCWCullBack);
        
        scene_->drawAll (d3d11.contextIM);

        d3d11.swapchain->Present (0u, 0u);
    }

    void setupMouseListener()
    {
        MessageManagerLock lock;
        fpsCamControl_ = new FPSCameraControl (cam_);
        addMouseListener (fpsCamControl_, false);
    }

    class FPSCameraControl : public MouseListener
    {
    private:
        FPSCameraControl (const FPSCameraControl&);
        void operator = (const FPSCameraControl&);
    public:
        
        enum State
        {
            stateIdle,
            stateDrag,
        };

        Camera& camera;
        bool active;
        State state;
        Transform3f startTransform;
        Point<float> startPoint;
        SyncWithAtomic sync;

        FPSCameraControl (Camera& camera)
            : camera (camera), active (true), state (stateIdle)
        {
        }

        void mouseDrag (const MouseEvent& evt) override
        {
            (void)evt;
            if (state != stateDrag)
                return;

            ScopedSyncWrite<SyncWithAtomic> lock (sync);

            Point<float> delta = evt.position - startPoint;

            //m_dprint_begin() << delta.getX() << ", " << delta.getY() << m_dprint_end();

            delta.getX();
            delta.getY();
            Vec3f rot;
            rot.x = (delta.getY() /-512.0f) * Scalar::cPI<float>();
            rot.y = (delta.getX() / 512.0f) * Scalar::cPI<float>();

            Vec4f q = Quat::fromXYZRotation (rot);

            Transform3f tran = camera.transform.fetch();
            tran.rotation = Quat::mul (startTransform.rotation, q);
            camera.transform.commit (tran);

            //Logger::outputDebugString (str);
        }

        void mouseDown (const MouseEvent& evt) override
        {
            (void)evt;
            if (!active)
                return;

            startTransform = camera.transform.fetch();
            startPoint = evt.position;

            state = stateDrag;
            m_dprint_begin() << "start dragging..." << m_dprint_end();
        }

        void mouseUp (const MouseEvent& evt) override
        {
            (void)evt;
            state = stateIdle;
            m_dprint_begin() << "stop dragging..." << m_dprint_end();
        }
    };

    Hold<ID3D11Buffer> cbFrameData_;
    ScopedPointer<D3D11Scene> scene_;
    ScopedPointer<FPSCameraControl> fpsCamControl_;
    Camera cam_;
};

//==============================================================================
class TheDemoApplication  : public DemoApplication<TheDemo>
{
public:
    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (TheDemoApplication)
