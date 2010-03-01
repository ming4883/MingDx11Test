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
	struct VSPreObjectStruct
	{
		D3DXMATRIX m_WorldViewProj;
		D3DXMATRIX m_World;
	};

	typedef js::ConstantBuffer_t<VSPreObjectStruct> VSPreObjectConstBuf;

	struct PSPreObjectStruct
	{
		D3DXVECTOR4 m_vObjectColor;
	};

	typedef js::ConstantBuffer_t<PSPreObjectStruct> PSPreObjectConstBuf;

#pragma pack(pop)
//#pragma pack(show)

// Global Variables
	CModelViewerCamera m_Camera;
	RenderableMesh m_Mesh;
	D3DXMATRIX m_World;
	D3DXMATRIX m_WorldViewProjection;
	std::auto_ptr<VSPreObjectConstBuf> m_VsPreObjectConstBuf;
	std::auto_ptr<PSPreObjectConstBuf> m_PsPreObjectConstBuf;
	ID3D11SamplerState* m_SamplerState;
	js::Texture2DRenderBuffer m_ColorBuffer;
	js::Texture2DRenderBuffer m_DepthBuffer;

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

		m_ColorBuffer.create(d3dDevice, backBufferSurfaceDesc->Width, backBufferSurfaceDesc->Height, 1, DXGI_FORMAT_R8G8B8A8_UNORM);
		js_assert(m_ColorBuffer.valid());

		m_DepthBuffer.create(d3dDevice, backBufferSurfaceDesc->Width, backBufferSurfaceDesc->Height, 1,
			DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);
		js_assert(m_DepthBuffer.valid());

		return S_OK;
	}

	__override void onD3D11ReleasingSwapChain()
	{
		m_ColorBuffer.destroy();
		m_DepthBuffer.destroy();
	}

	__override void onD3D11DestroyDevice()
	{
		DXUTGetGlobalResourceCache().OnDestroyDevice();
		m_Mesh.destroy();
		m_VsPreObjectConstBuf.reset();
		m_PsPreObjectConstBuf.reset();
		js_safe_release(m_SamplerState);
	}

	__override void onFrameMove(
		double time,
		float elapsedTime)
	{
		m_Camera.FrameMove( elapsedTime );

		D3DXMatrixIdentity(&m_World);
		m_WorldViewProjection = m_World * *m_Camera.GetViewMatrix() * *m_Camera.GetProjMatrix();
	}
	
	__override void onD3D11FrameRender(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dImmediateContext,
		double time,
		float elapsedTime)
	{
		// Clear render target and the depth stencil 
		static const float ClearColor[4] = { 0.176f, 0.176f, 0.176f, 0.0f };

		d3dImmediateContext->ClearRenderTargetView( DXUTGetD3D11RenderTargetView(), ClearColor );
		d3dImmediateContext->ClearDepthStencilView( DXUTGetD3D11DepthStencilView(), D3D11_CLEAR_DEPTH, 1.0, 0 );

		// m_VsPreObjectConstBuf
		m_VsPreObjectConstBuf->map(d3dImmediateContext);
		D3DXMatrixTranspose( &m_VsPreObjectConstBuf->data().m_WorldViewProj, &m_WorldViewProjection );
		D3DXMatrixTranspose( &m_VsPreObjectConstBuf->data().m_World, &m_World );
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


