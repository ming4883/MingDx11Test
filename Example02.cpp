#include "DXUT.h"

#include "SDKmisc.h"
#include "Common.h"
#include "DXUTcamera.h"
#include "Jessye/Shaders.h"
#include "Jessye/Buffers.h"

class Example02 : public DXUTApp
{
public:
// Data structures
#pragma pack(push)
//#pragma pack(show)
#pragma pack(1)
//#pragma pack(show)
	struct VSPreObject
	{
		D3DXMATRIX m_ViewProjection;
		D3DXMATRIX m_World;
		D3DXVECTOR3 m_CameraPosition;

		void update(const CModelViewerCamera& camera, const D3DXMATRIX& worldMatrix)
		{
			D3DXMATRIX viewProjectionMatrix = *camera.GetViewMatrix() * *camera.GetProjMatrix();
			D3DXMatrixTranspose(&m_ViewProjection, &viewProjectionMatrix);
			D3DXMatrixTranspose(&m_World, &worldMatrix);
			m_CameraPosition = *camera.GetEyePt();
		}
	};

	typedef js::ConstantBuffer_t<VSPreObject> VSPreObjectConstBuf;

	struct PSPreObject
	{
		D3DXVECTOR4 m_vObjectColor;
	};

	typedef js::ConstantBuffer_t<PSPreObject> PSPreObjectConstBuf;

	struct PSPostProcess
	{
		D3DXMATRIX m_InvViewProjScaleBias;
		D3DXVECTOR4 m_ZParams;
	};

	typedef js::ConstantBuffer_t<PSPostProcess> PSPostProcessConstBuf;

#pragma pack(pop)
//#pragma pack(show)

	class Rendering
	{
	public:
		js::Texture2DRenderBuffer m_ColorBuffer;
		js::Texture2DRenderBuffer m_DepthBuffer;

		void onSwapChainResized(ID3D11Device* d3dDevice, size_t width, size_t height)
		{
			m_ColorBuffer.create(d3dDevice, width, height, 1, DXGI_FORMAT_R8G8B8A8_UNORM);
			js_assert(m_ColorBuffer.valid());

			m_DepthBuffer.create(d3dDevice, width, height, 1,
				DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);
			js_assert(m_DepthBuffer.valid());
		}

		void onSwapChainReleasing()
		{
			m_ColorBuffer.destroy();
			m_DepthBuffer.destroy();
		}

		void prepareRenderScene(
			ID3D11Device* d3dDevice,
			ID3D11DeviceContext* d3dContext)
		{
			// Clear render target and the depth stencil 
			static const float ClearColor[4] = { 0.176f, 0.176f, 0.176f, 0.0f };

			d3dContext->ClearRenderTargetView( m_ColorBuffer.m_RTView, ClearColor );
			d3dContext->ClearDepthStencilView( m_DepthBuffer.m_DSView, D3D11_CLEAR_DEPTH, 1.0, 0 );
			d3dContext->OMSetRenderTargets(1, &m_ColorBuffer.m_RTView, m_DepthBuffer.m_DSView);
		}

		void unprepareRenderScene(
			ID3D11Device* d3dDevice,
			ID3D11DeviceContext* d3dContext)
		{
			ID3D11RenderTargetView* rtv[] = {DXUTGetD3D11RenderTargetView()};
			d3dContext->OMSetRenderTargets(1, rtv, DXUTGetD3D11DepthStencilView());
		}
	};

// Global Variables
	CModelViewerCamera m_Camera;
	RenderableMesh m_Mesh;
	D3DXMATRIX m_WorldMatrix;
	std::auto_ptr<VSPreObjectConstBuf> m_VsPreObjectConstBuf;
	std::auto_ptr<PSPreObjectConstBuf> m_PsPreObjectConstBuf;
	std::auto_ptr<PSPostProcessConstBuf> m_PSPostProcessConstBuf;
	ID3D11SamplerState* m_SamplerState;
	Rendering m_Rendering;
	

	// post processing
	ScreenQuad m_ScreenQuad;
	js::VertexShader m_PostVtxShd;
	js::PixelShader m_PostFogShd;

// Methods
	Example02()
		: m_SamplerState(nullptr)
	{
	}

	__override ~Example02()
	{
	}

	__override const wchar_t* getName() { return L"Example02"; }

	__override HRESULT onD3D11CreateDevice(
		ID3D11Device* d3dDevice,
		const DXGI_SURFACE_DESC* backBufferSurfaceDesc)
	{
		{	D3D11_SAMPLER_DESC sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			d3dDevice->CreateSamplerState(&sd, &m_SamplerState);
		}

		// load mesh
		RenderableMesh::ShaderDesc sd;
		sd.vsPath = media(L"Example02/Scene.VS.hlsl");
		sd.vsEntry = "VSMain";

		sd.psPath = media(L"Example02/Scene.PS.hlsl");
		sd.psEntry = "PSMain";

		std::vector<D3D11_INPUT_ELEMENT_DESC> ielems;
		inputElement(ielems, "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0);
		inputElement(ielems, "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0);
		inputElement(ielems, "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0);

		m_Mesh.create(d3dDevice, media(L"Common/SoftParticles/TankScene.sdkmesh"), sd, ielems);

		m_VsPreObjectConstBuf.reset(new VSPreObjectConstBuf);
		m_VsPreObjectConstBuf->create(d3dDevice);

		m_PsPreObjectConstBuf.reset(new PSPreObjectConstBuf);
		m_PsPreObjectConstBuf->create(d3dDevice);

		// post processing
		m_PostVtxShd.createFromFile(d3dDevice, media(L"Example02/Post.Vtx.hlsl"), "Main");
		js_assert(m_PostVtxShd.valid());

		m_PostFogShd.createFromFile(d3dDevice, media(L"Example02/Post.Fog.hlsl"), "Main");
		js_assert(m_PostFogShd.valid());

		m_ScreenQuad.create(d3dDevice, m_PostVtxShd.m_ByteCode);
		js_assert(m_ScreenQuad.valid());

		m_PSPostProcessConstBuf.reset(new PSPostProcessConstBuf);
		m_PSPostProcessConstBuf->create(d3dDevice);

		// init camera
		const float radius = m_Mesh.radius();
		D3DXVECTOR3 vecEye( 0.0f, 0.0f, -100.0f );
		D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
		m_Camera.SetViewParams( &vecEye, &vecAt );
		m_Camera.SetRadius(radius, radius * 0.5f, radius * 4);
		
		return S_OK;
	}

	__override HRESULT onD3D11ResizedSwapChain(
		ID3D11Device* d3dDevice,
		IDXGISwapChain* swapChain,
		const DXGI_SURFACE_DESC* backBufferSurfaceDesc)
	{
		float aspect = backBufferSurfaceDesc->Width / ( FLOAT )backBufferSurfaceDesc->Height;
		const float radius = m_Mesh.radius();

		m_Camera.SetProjParams( D3DX_PI / 4, aspect, 1.0f, radius * 8.0f );
		m_Camera.SetWindow( backBufferSurfaceDesc->Width, backBufferSurfaceDesc->Height );
		m_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );

		m_Rendering.onSwapChainResized(d3dDevice, backBufferSurfaceDesc->Width, backBufferSurfaceDesc->Height);

		return S_OK;
	}

	__override void onD3D11ReleasingSwapChain()
	{
		m_Rendering.onSwapChainReleasing();
	}

	__override void onD3D11DestroyDevice()
	{
		DXUTGetGlobalResourceCache().OnDestroyDevice();
		m_Mesh.destroy();
		m_VsPreObjectConstBuf.reset();
		m_PsPreObjectConstBuf.reset();

		m_PSPostProcessConstBuf.reset();
		
		m_ScreenQuad.destroy();
		m_PostVtxShd.destroy();
		m_PostFogShd.destroy();
		
		js_safe_release(m_SamplerState);
	}

	__override void onFrameMove(
		double time,
		float elapsedTime)
	{
		m_Camera.FrameMove( elapsedTime );
		D3DXMatrixIdentity(&m_WorldMatrix);
	}
	
	__override void onD3D11FrameRender(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dImmediateContext,
		double time,
		float elapsedTime)
	{
		D3DXMATRIX viewProjection = *m_Camera.GetViewMatrix() * *m_Camera.GetProjMatrix();

		// Render Scene
		{	m_Rendering.prepareRenderScene(d3dDevice, d3dImmediateContext);

			// m_VsPreObjectConstBuf
			m_VsPreObjectConstBuf->map(d3dImmediateContext);
			m_VsPreObjectConstBuf->data().update(m_Camera, m_WorldMatrix);
			m_VsPreObjectConstBuf->unmap(d3dImmediateContext);

			// m_PsPreObjectConstBuf
			m_PsPreObjectConstBuf->map(d3dImmediateContext);
			m_PsPreObjectConstBuf->data().m_vObjectColor = D3DXVECTOR4(1, 1, 0.5f, 1);
			m_PsPreObjectConstBuf->unmap(d3dImmediateContext);

			// preparing shaders
			d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_VsPreObjectConstBuf->m_BufferObject);
			d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_PsPreObjectConstBuf->m_BufferObject);
			d3dImmediateContext->PSSetSamplers(0, 1, &m_SamplerState);

			m_Mesh.render(d3dImmediateContext);

		}	m_Rendering.unprepareRenderScene(d3dDevice, d3dImmediateContext);

		// Post-Processing
		{	
			// todo replace this with disabling depth test
			d3dImmediateContext->ClearDepthStencilView( DXUTGetD3D11DepthStencilView(), D3D11_CLEAR_DEPTH, 1.0, 0 );

			{	// m_PSPostProcessConstBuf
				PSPostProcessConstBuf& buf = *m_PSPostProcessConstBuf;
				buf.map(d3dImmediateContext);
				
				D3DXMATRIX scalebias(
					 2 / (float)DXUTGetDXGIBackBufferSurfaceDesc()->Width, 0, 0, 0,
					 0, 2 / (float)DXUTGetDXGIBackBufferSurfaceDesc()->Height, 0, 0,
					 0, 0, 1, 0,
					-1,-1, 0, 1);

				D3DXMATRIX invViewProj;
				D3DXMatrixInverse(&invViewProj, nullptr, &viewProjection);
				invViewProj = scalebias * invViewProj;

				D3DXMatrixTranspose(&buf.data().m_InvViewProjScaleBias, &invViewProj);

				buf.data().m_ZParams.x = 1 / m_Camera.GetFarClip() - 1 / m_Camera.GetNearClip();
				buf.data().m_ZParams.y = 1 / m_Camera.GetNearClip();
				buf.data().m_ZParams.z = m_Camera.GetFarClip() - m_Camera.GetNearClip();
				buf.data().m_ZParams.w = m_Camera.GetNearClip();

				buf.unmap(d3dImmediateContext);
			}

			d3dImmediateContext->VSSetShader(m_PostVtxShd.m_ShaderObject, nullptr, 0);
			d3dImmediateContext->PSSetShader(m_PostFogShd.m_ShaderObject, nullptr, 0);

			{	ID3D11ShaderResourceView* shv[] = {
					m_Rendering.m_ColorBuffer.m_SRView, 
					m_Rendering.m_DepthBuffer.m_SRView
					};
				d3dImmediateContext->PSSetShaderResources(0, 2, shv);
				d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_PSPostProcessConstBuf->m_BufferObject);
			}

			m_ScreenQuad.render(d3dImmediateContext);

			{
				ID3D11ShaderResourceView* shv[] = {nullptr, nullptr};
				d3dImmediateContext->PSSetShaderResources(0, 2, shv);
			}
		}
	}

	__override LRESULT msgProc(
		HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing)
	{
		m_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);
		return 0;
	}

};	// Example02

// Initialize everything and go into a render loop
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	Example02 app;
	return DXUTApp::run(app);
}


