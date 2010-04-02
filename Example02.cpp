#include "DXUT.h"

#include "SDKmisc.h"
#include "Common.h"
#include "DXUTcamera.h"
#include "Jessye/Shaders.h"
#include "Jessye/Buffers.h"
#include "Jessye/RenderStates.h"
#include "Jessye/ViewArrays.h"


class Histogram
{
public:
	
	enum {SIZE = 16};

#pragma pack(push)
#pragma pack(1)
	struct HistogramCompute_s
	{
		D3DXVECTOR4 g_vInputParams;
	};

	typedef js::ConstantBuffer_t<HistogramCompute_s> HistogramComputeConstBuf;

	struct HistogramDraw_s
	{
		D3DXVECTOR4 g_vDrawParams;
	};

	typedef js::ConstantBuffer_t<HistogramDraw_s> HistogramDrawConstBuf;

#pragma pack(pop)
	js::Texture2DRenderBuffer m_Buffer;
	js::VertexShader m_ComputeVs;
	js::GeometryShader m_ComputeGs;
	js::PixelShader m_ComputePs;
	js::VertexShader m_DrawVs;
	js::GeometryShader m_DrawGs;
	js::PixelShader m_DrawPs;
	HistogramComputeConstBuf m_ComputeCb;
	HistogramDrawConstBuf m_DrawCb;
	ID3D11Buffer* m_VB;
	ID3D11InputLayout* m_IL;

	float m_LastNumInputs;
	float m_MaxInputValue;
	
	Histogram() : m_MaxInputValue(4.0f), m_LastNumInputs(1)
	{
	}

	void create(ID3D11Device* d3dDevice)
	{
		m_Buffer.create(d3dDevice, SIZE, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);

		m_ComputeVs.createFromFile(d3dDevice, media(L"Example02/Histogram.Compute.VS.hlsl"), "Main");
		js_assert(m_ComputeVs.valid());
		
		m_ComputeGs.createFromFile(d3dDevice, media(L"Example02/Histogram.Compute.GS.hlsl"), "Main");
		js_assert(m_ComputeGs.valid());

		m_ComputePs.createFromFile(d3dDevice, media(L"Example02/Histogram.Compute.PS.hlsl"), "Main");
		js_assert(m_ComputePs.valid());

		m_DrawVs.createFromFile(d3dDevice, media(L"Example02/Histogram.Draw.VS.hlsl"), "Main");
		js_assert(m_DrawVs.valid());

		m_DrawGs.createFromFile(d3dDevice, media(L"Example02/Histogram.Draw.GS.hlsl"), "Main");
		js_assert(m_DrawGs.valid());

		m_DrawPs.createFromFile(d3dDevice, media(L"Example02/Histogram.Draw.PS.hlsl"), "Main");
		js_assert(m_DrawPs.valid());

		m_ComputeCb.create(d3dDevice);
		js_assert(m_ComputeCb.valid());
		
		m_DrawCb.create(d3dDevice);
		js_assert(m_DrawCb.valid());

		static D3DXVECTOR3 v[] = {D3DXVECTOR3(0,0,0)};
		m_VB = js::Buffers::createVertexBuffer(d3dDevice, sizeof(v), sizeof(v[0]), false, v);
		js_assert(m_VB != nullptr);

		std::vector<D3D11_INPUT_ELEMENT_DESC> ielems;
		inputElement(ielems, "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0);

		m_IL = js::Buffers::createInputLayout(d3dDevice, &ielems[0], ielems.size(), m_ComputeVs.m_ByteCode);
		js_assert(m_IL != nullptr);
	}

	void destroy()
	{
		m_Buffer.destroy();
		m_ComputeVs.destroy();
		m_ComputeGs.destroy();
		m_ComputePs.destroy();
		m_DrawVs.destroy();
		m_DrawGs.destroy();
		m_DrawPs.destroy();
		m_ComputeCb.destroy();
		m_DrawCb.destroy();
		js_safe_release(m_VB);
		js_safe_release(m_IL);
	}

	void update(ID3D11DeviceContext* d3dContext, js::RenderStateCache& rsCache, js::Texture2DRenderBuffer& colorBuffer)
	{
		js_assert(colorBuffer.valid());

		m_ComputeCb.map(d3dContext);
		m_ComputeCb.data().g_vInputParams.x = (float)(colorBuffer.m_Width / 4);
		m_ComputeCb.data().g_vInputParams.y = 4;
		m_ComputeCb.data().g_vInputParams.z = m_MaxInputValue;
		m_ComputeCb.unmap(d3dContext);

		// vertex buffer and input assembler
		d3dContext->IASetInputLayout(m_IL);
		d3dContext->IASetVertexBuffers(0, 1, &m_VB, js::UintVA() << sizeof(D3DXVECTOR3), js::UintVA() << 0);
		d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		
		// render target
		static const float clearColor[] = {0, 0, 0, 0};
		d3dContext->ClearRenderTargetView(m_Buffer.m_RTView, clearColor);
		size_t vpCnt; D3D11_VIEWPORT vps[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		d3dContext->RSGetViewports(&vpCnt, nullptr);
		d3dContext->RSGetViewports(&vpCnt, vps);
		d3dContext->RSSetViewports(1, js::VpVA() << m_Buffer.viewport());

		rsCache.renderTarget().backup();
		rsCache.renderTarget().set(1, js::RtvVA() << m_Buffer, nullptr);

		// render state
		rsCache.blendState().backup();
		rsCache.blendState().RenderTarget[0].BlendEnable = TRUE;
		rsCache.blendState().RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		rsCache.blendState().RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rsCache.blendState().RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		rsCache.blendState().RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		rsCache.blendState().RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		rsCache.blendState().RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		rsCache.blendState().RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		rsCache.blendState().dirty();

		// shader state
		rsCache.vertexShader().backup();
		rsCache.vertexShader().setShader(m_ComputeVs);

		rsCache.geometryShader().backup();
		rsCache.geometryShader().setShader(m_ComputeGs);
		rsCache.geometryShader().setSRViews(0, 1, js::SrvVA() << colorBuffer);
		rsCache.geometryShader().setConstBuffers(0, 1, js::BufVA() << m_ComputeCb);

		rsCache.pixelShader().backup();
		rsCache.pixelShader().setShader(m_ComputePs);
		rsCache.applyToContext(d3dContext);

		// draw
		d3dContext->DrawInstanced(1, (colorBuffer.m_Width * colorBuffer.m_Height) / 16, 0, 0);

		m_LastNumInputs = (float)(colorBuffer.m_Width * colorBuffer.m_Height);
		
		// restore render targets
		d3dContext->RSSetViewports(vpCnt, vps);

		rsCache.renderTarget().restore();
		rsCache.blendState().restore();
		rsCache.vertexShader().restore();
		rsCache.geometryShader().restore();
		rsCache.pixelShader().restore();
	}

	void display(ID3D11DeviceContext* d3dContext, js::RenderStateCache& rsCache, float x, float y, float w, float h)
	{
		m_DrawCb.map(d3dContext);
		m_DrawCb.data().g_vDrawParams.x = x;
		m_DrawCb.data().g_vDrawParams.y = y;
		m_DrawCb.data().g_vDrawParams.z = w / SIZE;
		m_DrawCb.data().g_vDrawParams.w = h / m_LastNumInputs;
		m_DrawCb.unmap(d3dContext);
		
		// vertex buffer and input assembler
		d3dContext->IASetInputLayout(m_IL);
		d3dContext->IASetVertexBuffers(0, 1, &m_VB, js::UintVA() << sizeof(D3DXVECTOR3), js::UintVA() << 0);
		d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		//shader states
		rsCache.vertexShader().backup();
		rsCache.vertexShader().setShader(m_DrawVs);
		rsCache.vertexShader().setSRViews(0, 1, js::SrvVA() << m_Buffer);
		rsCache.vertexShader().setConstBuffers(0, 1, js::BufVA() << m_DrawCb);

		rsCache.geometryShader().backup();
		rsCache.geometryShader().setShader(m_DrawGs);
		rsCache.geometryShader().setConstBuffers(0, 1, js::BufVA() << m_DrawCb);

		rsCache.pixelShader().backup();
		rsCache.pixelShader().setShader(m_DrawPs);

		rsCache.applyToContext(d3dContext);

		// draw
		d3dContext->DrawInstanced(1, SIZE, 0, 0);

		// restore shader resources
		rsCache.vertexShader().restore();
		rsCache.geometryShader().restore();
		rsCache.pixelShader().restore();
	}
};	// Histogram

class Example02 : public DXUTApp
{
public:
// Data structures
#pragma pack(push)
#pragma pack(1)
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

		void update(const CModelViewerCamera& camera)
		{
			D3DXMATRIX scalebias(
				 2 / (float)DXUTGetDXGIBackBufferSurfaceDesc()->Width, 0, 0, 0,
				 0, 2 / (float)DXUTGetDXGIBackBufferSurfaceDesc()->Height, 0, 0,
				 0, 0, 1, 0,
				-1,-1, 0, 1);

			D3DXMATRIX viewProjectionMatrix = *camera.GetViewMatrix() * *camera.GetProjMatrix();
			D3DXMATRIX invViewProj;
			D3DXMatrixInverse(&invViewProj, nullptr, &viewProjectionMatrix);
			invViewProj = scalebias * invViewProj;

			D3DXMatrixTranspose(&m_InvViewProjScaleBias, &invViewProj);

			m_ZParams.x = 1 / camera.GetFarClip() - 1 / camera.GetNearClip();
			m_ZParams.y = 1 / camera.GetNearClip();
			m_ZParams.z = camera.GetFarClip() - camera.GetNearClip();
			m_ZParams.w = camera.GetNearClip();
		}
	};

	typedef js::ConstantBuffer_t<PSPostProcess> PSPostProcessConstBuf;

#pragma pack(pop)

// Global Variables
	CModelViewerCamera m_Camera;
	RenderableMesh m_Mesh;
	D3DXMATRIX m_WorldMatrix;

	js::Texture2DRenderBuffer m_ColorBuffer;
	js::Texture2DRenderBuffer m_ColorBufferQuater;
	js::Texture2DRenderBuffer m_DepthBuffer;

	VSPreObjectConstBuf m_VsPreObjectConstBuf;
	PSPreObjectConstBuf m_PsPreObjectConstBuf;
	PSPostProcessConstBuf m_PSPostProcessConstBuf;
	js::SamplerState m_SamplerState;

	//Rendering m_Rendering;
	Histogram m_Histogram;
	bool m_ShowHistogram;

	js::RenderStateCache m_RSCache;
	
	// post processing
	ScreenQuad m_ScreenQuad;
	js::VertexShader m_PostVtxShd;
	js::PixelShader m_PostFogShd;

// Methods
	Example02()
		: m_ShowHistogram(false)
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
		guiOnD3D11CreateDevice(d3dDevice);

		m_RSCache.create(d3dDevice);

		// scene
		m_SamplerState.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerState.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerState.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerState.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		m_SamplerState.create(d3dDevice);

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

		m_VsPreObjectConstBuf.create(d3dDevice);
		m_PsPreObjectConstBuf.create(d3dDevice);

		m_Histogram.create(d3dDevice);

		// post processing
		m_PostVtxShd.createFromFile(d3dDevice, media(L"Example02/Post.Vtx.hlsl"), "Main");
		js_assert(m_PostVtxShd.valid());

		m_PostFogShd.createFromFile(d3dDevice, media(L"Example02/Post.DOF.hlsl"), "Main");
		js_assert(m_PostFogShd.valid());

		m_ScreenQuad.create(d3dDevice, m_PostVtxShd.m_ByteCode);
		js_assert(m_ScreenQuad.valid());

		m_PSPostProcessConstBuf.create(d3dDevice);

		// init camera
		const float radius = m_Mesh.radius();
		D3DXVECTOR3 vecEye( 0.0f, 0.0f, -100.0f );
		D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
		m_Camera.SetViewParams( &vecEye, &vecAt );
		m_Camera.SetRadius(radius, radius * 0.25f, radius * 4);
		
		return S_OK;
	}

	__override HRESULT onD3D11ResizedSwapChain(
		ID3D11Device* d3dDevice,
		IDXGISwapChain* swapChain,
		const DXGI_SURFACE_DESC* backBufferSurfaceDesc)
	{
		guiOnD3D11ResizedSwapChain(d3dDevice, backBufferSurfaceDesc);

		float aspect = backBufferSurfaceDesc->Width / ( FLOAT )backBufferSurfaceDesc->Height;
		const float radius = m_Mesh.radius();

		const size_t width = backBufferSurfaceDesc->Width;
		const size_t height = backBufferSurfaceDesc->Height;

		m_Camera.SetProjParams( D3DX_PI / 4, aspect, 1.0f, radius * 8.0f );
		m_Camera.SetWindow( width, height );
		m_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );

		// create render buffers
		m_ColorBuffer.create(d3dDevice, width, height, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
		js_assert(m_ColorBuffer.valid());
		
		m_ColorBufferQuater.create(d3dDevice, width / 4, height / 4, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
		js_assert(m_ColorBufferQuater.valid());

		m_DepthBuffer.create(d3dDevice, width, height, 1,
			DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
		js_assert(m_DepthBuffer.valid());

		return S_OK;
	}

	__override void onD3D11ReleasingSwapChain()
	{
		guiOnD3D11ReleasingSwapChain();

		// destroy render buffers
		m_ColorBuffer.destroy();
		m_ColorBufferQuater.destroy();
		m_DepthBuffer.destroy();
	}

	__override void onD3D11DestroyDevice()
	{
		guiOnD3D11DestroyDevice();

		m_RSCache.destroy();

		DXUTGetGlobalResourceCache().OnDestroyDevice();
		m_Mesh.destroy();
		m_VsPreObjectConstBuf.destroy();
		m_PsPreObjectConstBuf.destroy();

		m_Histogram.destroy();

		m_PSPostProcessConstBuf.destroy();
		
		m_ScreenQuad.destroy();
		m_PostVtxShd.destroy();
		m_PostFogShd.destroy();
		
		m_SamplerState.destroy();
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
		m_RSCache.renderTarget().set(1, js::RtvVA() << DXUTGetD3D11RenderTargetView(), DXUTGetD3D11DepthStencilView());

		m_RSCache.renderTarget().backup();
		m_RSCache.vertexShader().backup();
		m_RSCache.pixelShader().backup();
		m_RSCache.geometryShader().backup();

		// Scene
		onD3D11FrameRender_ClearRenderTargets(d3dImmediateContext);
		m_RSCache.renderTarget().set(1, js::RtvVA() << m_ColorBuffer, m_DepthBuffer);

		onD3D11FrameRender_Scene_PrepareShaderResources(d3dImmediateContext);
		onD3D11FrameRender_Scene_DrawMesh(d3dImmediateContext);

		m_RSCache.vertexShader().restore();
		m_RSCache.pixelShader().restore();
		m_RSCache.geometryShader().restore();
		m_RSCache.renderTarget().restore();

		// Post-Processing
		onD3D11FrameRender_PostProcessing(d3dImmediateContext);

		// Gui
		m_GuiTxtHelper->Begin();
		m_GuiTxtHelper->SetInsertionPos( 2, 0 );
		m_GuiTxtHelper->SetForegroundColor( D3DXCOLOR( 0.2f, 0.2f, 1.0f, 1.0f ) );
		m_GuiTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
		m_GuiTxtHelper->End();
	}

	void onD3D11FrameRender_ClearRenderTargets(ID3D11DeviceContext* d3dContext)
	{
		// clear render target and the depth stencil 
		static const float clearColor[4] = { 2, 2, 2, 0 };

		d3dContext->ClearRenderTargetView( m_ColorBuffer.m_RTView, clearColor );
		d3dContext->ClearDepthStencilView( m_DepthBuffer.m_DSView, D3D11_CLEAR_DEPTH, 1.0, 0 );
	}

	void onD3D11FrameRender_Scene_PrepareShaderResources(ID3D11DeviceContext* d3dContext)
	{
		// m_VsPreObjectConstBuf
		m_VsPreObjectConstBuf.map(d3dContext);
		m_VsPreObjectConstBuf.data().update(m_Camera, m_WorldMatrix);
		m_VsPreObjectConstBuf.unmap(d3dContext);

		// m_PsPreObjectConstBuf
		m_PsPreObjectConstBuf.map(d3dContext);
		m_PsPreObjectConstBuf.data().m_vObjectColor = D3DXVECTOR4(1, 1, 0.5f, 1);
		m_PsPreObjectConstBuf.unmap(d3dContext);

		// preparing shaders
		m_RSCache.vertexShader().setConstBuffers(0, 1, js::BufVA() << m_VsPreObjectConstBuf);
		m_RSCache.pixelShader().setConstBuffers(0, 1, js::BufVA() << m_PsPreObjectConstBuf);
		m_RSCache.pixelShader().setSamplers(0, 1, js::SampVA() << m_SamplerState);
	}
	
	void onD3D11FrameRender_Scene_DrawMesh(ID3D11DeviceContext* d3dContext)
	{
		m_RSCache.applyToContext(d3dContext);
		m_Mesh.render(d3dContext, &m_RSCache);
	}
	
	void onD3D11FrameRender_PostProcessing(ID3D11DeviceContext* d3dContext)
	{
		m_PSPostProcessConstBuf.map(d3dContext);
		m_PSPostProcessConstBuf.data().update(m_Camera);
		m_PSPostProcessConstBuf.unmap(d3dContext);

		// depth stencil state
		m_RSCache.depthStencilState().backup();
		m_RSCache.depthStencilState().DepthEnable = FALSE;
		m_RSCache.depthStencilState().DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		m_RSCache.depthStencilState().dirty();

		// update histogram
		if(m_ShowHistogram)
			m_Histogram.update(d3dContext, m_RSCache, m_ColorBuffer);
		
		// shader states
		m_RSCache.vertexShader().backup();
		m_RSCache.vertexShader().setShader(m_PostVtxShd);

		m_RSCache.pixelShader().backup();
		m_RSCache.pixelShader().setShader(m_PostFogShd);
		m_RSCache.pixelShader().setSRViews(0, 2, js::SrvVA() << m_ColorBuffer << m_DepthBuffer);
		m_RSCache.pixelShader().setConstBuffers(0, 1, js::BufVA() << m_PSPostProcessConstBuf);

		m_RSCache.applyToContext(d3dContext);
		
		m_ScreenQuad.render(d3dContext);

		// restore shader states
		m_RSCache.vertexShader().restore();
		m_RSCache.pixelShader().restore();

		if(m_ShowHistogram)
			m_Histogram.display(d3dContext, m_RSCache, -0.9f, -0.5f, 0.5f, 0.25f);

		// restore depth stencil state
		m_RSCache.depthStencilState().restore();
	}

	__override LRESULT msgProc(
		HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing)
	{
		m_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	__override void onKeyboard(UINT nChar, bool keyDown, bool altDown)
	{
		if(VK_F1 == nChar && keyDown)
		{
			m_ShowHistogram = !m_ShowHistogram;
		}
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


