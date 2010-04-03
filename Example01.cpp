#include "DXUT.h"

#include "SDKmisc.h"
#include "Common.h"
#include "DXUTcamera.h"
#include "Jessye/Shaders.h"
#include "Jessye/Buffers.h"
#include "Jessye/RenderStates.h"
#include "Jessye/ViewArrays.h"

class Example01 : public DXUTApp
{
public:
	enum { NUM_INSTANCES = 4 };
// Data structures
#pragma pack(push)
#pragma pack(1)
	struct VSDefault
	{
		D3DXMATRIX m_ViewProjection;
		D3DXMATRIX m_World;

		void update(const D3DXMATRIX& viewProjMatrix, const D3DXMATRIX& worldMatrix)
		{
			D3DXMatrixTranspose(&m_ViewProjection, &viewProjMatrix);
			D3DXMatrixTranspose(&m_World, &worldMatrix);
		}
	};

	typedef js::ConstantBuffer_t<VSDefault> VSDefaultConstBuf;

	struct VSInstancing
	{
		D3DXMATRIX m_InstanceWorld[NUM_INSTANCES];

		void update(const D3DXVECTOR3* instanceData)
		{
			for(int i=0; i<NUM_INSTANCES; ++i)
			{
				D3DXMATRIX* m = &m_InstanceWorld[i];
				D3DXMatrixIdentity(m);
				D3DXMatrixTranslation(m, instanceData[i].x, instanceData[i].y, instanceData[i].z);
				D3DXMatrixTranspose(m, m);
			}
		}
	};

	typedef js::ConstantBuffer_t<VSInstancing> VSInstancingConstBuf;

	struct PSDefault
	{
		D3DXVECTOR4 m_vObjectColor;

		void update(const D3DXVECTOR4& objColor)
		{
			m_vObjectColor = objColor;
		}
	};

	typedef js::ConstantBuffer_t<PSDefault> PSDefaultConstBuf;

#pragma pack(pop)

// Global Variables
	CModelViewerCamera m_Camera;
	RenderableMesh m_Mesh;
	D3DXMATRIX m_WorldMatrix;
	VSDefaultConstBuf m_VSDefaultConstBuf;
	VSInstancingConstBuf m_VSInstancingConstBuf;
	PSDefaultConstBuf m_PSDefaultConstBuf;
	js::SamplerState m_SamplerState;
	js::RenderStateCache m_RSCache;

// Methods
	Example01()
	{
	}

	__override ~Example01()
	{
	}

	__override const wchar_t* getName() { return L"Example01"; }

	__override HRESULT onD3D11CreateDevice(
		ID3D11Device* d3dDevice,
		const DXGI_SURFACE_DESC* backBufferSurfaceDesc)
	{
		m_SamplerState.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerState.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerState.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerState.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		m_SamplerState.create(d3dDevice);

		m_RSCache.create(d3dDevice);

		// load mesh
		RenderableMesh::ShaderDesc sd;
		sd.vsPath = media(L"Example01/BasicHLSL11_VS.hlsl");
		sd.vsEntry = "VSMain";

		sd.psPath = media(L"Example01/BasicHLSL11_PS.hlsl");
		sd.psEntry = "PSMain";

		std::vector<D3D11_INPUT_ELEMENT_DESC> ielems;
		inputElement(ielems, "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0);
		inputElement(ielems, "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0);
		inputElement(ielems, "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0);

		m_Mesh.create(d3dDevice, media(L"Common/Tiny/Tiny.sdkmesh"), sd, ielems);
		m_VSDefaultConstBuf.create(d3dDevice);
		m_VSInstancingConstBuf.create(d3dDevice);
		m_PSDefaultConstBuf.create(d3dDevice);

		// init camera
		const float radius = m_Mesh.radius();
		D3DXVECTOR3 vecEye( 0.0f, 0.0f, -100.0f );
		D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
		m_Camera.SetViewParams( &vecEye, &vecAt );
		m_Camera.SetRadius(radius * 2, radius * 0.5f, radius * 4);
		
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

		return S_OK;
	}

	__override void onD3D11ReleasingSwapChain()
	{
	}

	__override void onD3D11DestroyDevice()
	{
		DXUTGetGlobalResourceCache().OnDestroyDevice();
		m_Mesh.destroy();
		m_VSDefaultConstBuf.destroy();
		m_PSDefaultConstBuf.destroy();
		m_VSInstancingConstBuf.destroy();
		m_SamplerState.destroy();
		m_RSCache.destroy();
	}

	__override void onFrameMove(
		double time,
		float elapsedTime)
	{
		m_Camera.FrameMove( elapsedTime );

		D3DXMatrixIdentity(&m_WorldMatrix);
		D3DXMatrixRotationX(&m_WorldMatrix, D3DXToDegree(90));
	}
	
	__override void onD3D11FrameRender(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dImmediateContext,
		double time,
		float elapsedTime)
	{
		m_RSCache.rtState().set(1, js::RtvVA() << DXUTGetD3D11RenderTargetView(), DXUTGetD3D11DepthStencilView());

		onD3D11FrameRender_ClearRenderTargets(d3dImmediateContext);

		onD3D11FrameRender_PrepareShaderResources(d3dImmediateContext);

		onD3D11FrameRender_DrawMesh(d3dImmediateContext, false);
		onD3D11FrameRender_DrawMesh(d3dImmediateContext, true);
	}
	
	void onD3D11FrameRender_ClearRenderTargets(ID3D11DeviceContext* d3dContext)
	{
		// Clear render target and the depth stencil 
		static const float ClearColor[4] = { 0.176f, 0.176f, 0.176f, 0.0f };

		d3dContext->ClearRenderTargetView( DXUTGetD3D11RenderTargetView(), ClearColor );
		d3dContext->ClearDepthStencilView( DXUTGetD3D11DepthStencilView(), D3D11_CLEAR_DEPTH, 1.0, 0 );
	}

	void onD3D11FrameRender_PrepareShaderResources(ID3D11DeviceContext* d3dContext)
	{
		const D3DXMATRIX cViewProjMatrix = *m_Camera.GetViewMatrix() * *m_Camera.GetProjMatrix();

		static const float d = 150;
		static const D3DXVECTOR3 cInstanceData[NUM_INSTANCES] = {
			D3DXVECTOR3( 2*d, 0, 0),
			D3DXVECTOR3(   d, 0, 0),
			D3DXVECTOR3(-  d, 0, 0),
			D3DXVECTOR3(-2*d, 0, 0),
		};

		// m_VSDefaultConstBuf
		m_VSDefaultConstBuf.map(d3dContext);
		m_VSDefaultConstBuf.data().update(cViewProjMatrix, m_WorldMatrix);
		m_VSDefaultConstBuf.unmap(d3dContext);
		
		// m_VSInstancingConstBuf
		m_VSInstancingConstBuf.map(d3dContext);
		m_VSInstancingConstBuf.data().update(cInstanceData);
		m_VSInstancingConstBuf.unmap(d3dContext);

		// m_PSDefaultConstBuf
		m_PSDefaultConstBuf.map(d3dContext);
		m_PSDefaultConstBuf.data().m_vObjectColor = D3DXVECTOR4(1, 1, 0.5f, 1);
		m_PSDefaultConstBuf.unmap(d3dContext);

		// preparing shaders
		m_RSCache.vsState().setConstBuffers(0, 2, js::BufVA() << m_VSDefaultConstBuf << m_VSInstancingConstBuf);
		m_RSCache.psState().setConstBuffers(0, 1, js::BufVA() << m_PSDefaultConstBuf);
		m_RSCache.psState().setSamplers(0, 1, js::SampVA() << m_SamplerState);

	}

	void onD3D11FrameRender_DrawMesh(ID3D11DeviceContext* d3dContext, bool blend)
	{
		if(blend)
		{
			m_RSCache.blendState().backup();
			m_RSCache.blendState().RenderTarget[0].BlendEnable = TRUE;
			m_RSCache.blendState().RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			m_RSCache.blendState().RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			m_RSCache.blendState().RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			m_RSCache.blendState().RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			m_RSCache.blendState().dirty();

			m_RSCache.depthStencilState().backup();
			m_RSCache.depthStencilState().DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			m_RSCache.depthStencilState().dirty();

			m_RSCache.rasterizerState().backup();
			m_RSCache.rasterizerState().FillMode = D3D11_FILL_WIREFRAME;
			m_RSCache.rasterizerState().dirty();
		}

		m_RSCache.applyToContext(d3dContext);

		m_Mesh.render(d3dContext, &m_RSCache, NUM_INSTANCES);

		if(blend)
		{
			m_RSCache.blendState().restore();
			m_RSCache.depthStencilState().restore();
			m_RSCache.rasterizerState().restore();
		}
	}

	__override LRESULT msgProc(
		HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing)
	{
		m_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);
		return 0;
	}

};	// Example01

// Initialize everything and go into a render loop
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	Example01 app;
	return DXUTApp::run(app);
}


