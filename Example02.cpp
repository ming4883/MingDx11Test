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
	js::Texture2DStagingBuffer m_BufferStaging;
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
	float m_AdaptTime;
	float m_HistogramTarget;
	float m_BloomThreshold;
	D3DXVECTOR4 m_HDRParams; // min, max, key
	
	Histogram()
		: m_MaxInputValue(4.0f)
		, m_LastNumInputs(1)
		, m_AdaptTime(2)
		, m_HDRParams(0, 1, 1, 0)
		, m_HistogramTarget(0.95f)
		, m_BloomThreshold(0.5f)
	{
	}

	void create(ID3D11Device* d3dDevice)
	{
		m_Buffer.create(d3dDevice, SIZE, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		js_assert(m_Buffer.valid());

		m_BufferStaging.create(d3dDevice, SIZE, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		js_assert(m_BufferStaging.valid());

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
		m_BufferStaging.destroy();
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

	void update(ID3D11DeviceContext* d3dContext, js::RenderStateCache& rsCache, js::Texture2DRenderBuffer& colorBuffer, float dt)
	{
		js_assert(colorBuffer.valid());

		m_ComputeCb.map(d3dContext);
		m_ComputeCb.data().g_vInputParams.x = (float)(colorBuffer.m_Width / 4);
		m_ComputeCb.data().g_vInputParams.y = 4;
		m_ComputeCb.data().g_vInputParams.z = m_MaxInputValue;
		m_ComputeCb.data().g_vInputParams.w = (float)SIZE;
		m_ComputeCb.unmap(d3dContext);

		// vertex buffer and input assembler
		d3dContext->IASetInputLayout(m_IL);
		d3dContext->IASetVertexBuffers(0, 1, &m_VB, js::UintVA() << sizeof(D3DXVECTOR3), js::UintVA() << 0);
		d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		
		// render target
		static const float clearColor[] = {0, 0, 0, 0};
		d3dContext->ClearRenderTargetView(m_Buffer.m_RTView, clearColor);
		size_t vpCnt = 0; D3D11_VIEWPORT vps[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		d3dContext->RSGetViewports(&vpCnt, nullptr);
		d3dContext->RSGetViewports(&vpCnt, vps);
		d3dContext->RSSetViewports(1, js::VpVA() << m_Buffer.viewport());

		rsCache.rtState().backup();
		rsCache.rtState().set(1, js::RtvVA() << m_Buffer, nullptr);

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
		rsCache.vsState().backup();
		rsCache.vsState().setShader(m_ComputeVs);

		rsCache.gsState().backup();
		rsCache.gsState().setShader(m_ComputeGs);
		rsCache.gsState().setSRViews(0, 1, js::SrvVA() << colorBuffer);
		rsCache.gsState().setConstBuffers(0, 1, js::BufVA() << m_ComputeCb);

		rsCache.psState().backup();
		rsCache.psState().setShader(m_ComputePs);
		rsCache.applyToContext(d3dContext);

		// draw
		d3dContext->DrawInstanced(1, (colorBuffer.m_Width * colorBuffer.m_Height) / 16, 0, 0);

		m_LastNumInputs = (float)(colorBuffer.m_Width * colorBuffer.m_Height);
		
		// restore render targets
		d3dContext->RSSetViewports(vpCnt, vps);

		rsCache.rtState().restore();
		rsCache.blendState().restore();
		rsCache.vsState().restore();
		rsCache.gsState().restore();
		rsCache.psState().restore();

		// measure hdrParams
		d3dContext->CopyResource(m_BufferStaging.m_TextureObject, m_Buffer.m_TextureObject);
		m_BufferStaging.map(d3dContext, D3D11_MAP_READ);

		typedef float f4[4];
		f4* data = m_BufferStaging.data<f4>();
		
		float histogram[SIZE]; float histogramSum = 0;
		for(size_t i=0; i<SIZE; ++i)
		{
			histogram[i] = *data[0];
			histogramSum += histogram[i];
			++data;
		}
		m_BufferStaging.unmap(d3dContext);

		size_t minIdx = 0;
		while(histogram[minIdx] <= 0) {++minIdx;}

		size_t maxIdx = SIZE-1;
		while(histogram[maxIdx] <= 0) {--maxIdx;}

		size_t keyIdx = SIZE-1;
		float currentValue = 0;
		const float targetValue = histogramSum * (1-m_HistogramTarget);
		while((currentValue += histogram[keyIdx]) < targetValue) {--keyIdx;}

		float mean = 0;
		for(size_t i=0; i<SIZE; ++i)
			mean += histogram[i] * ((float)i / SIZE) * m_MaxInputValue;
		mean /= histogramSum;

		D3DXVECTOR4 hdrParams;
		hdrParams.x = mean;
		hdrParams.y = ((float)maxIdx / SIZE) * m_MaxInputValue;
		{
			float a = ((float)(keyIdx+1) / SIZE) * m_MaxInputValue;
			float b = ((float)(keyIdx+1) / SIZE) * m_MaxInputValue;
			float aVal = currentValue - histogram[keyIdx];
			float bVal = currentValue;
			float t = (targetValue - aVal) / (bVal) - (aVal);
			hdrParams.z = a + (b-a) * t;
			hdrParams.w = hdrParams.z * m_BloomThreshold;
		}

		const float t = dt * m_AdaptTime;
		m_HDRParams = m_HDRParams * (1 - t) + hdrParams * t;
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
		rsCache.vsState().backup();
		rsCache.vsState().setShader(m_DrawVs);
		rsCache.vsState().setSRViews(0, 1, js::SrvVA() << m_Buffer);
		rsCache.vsState().setConstBuffers(0, 1, js::BufVA() << m_DrawCb);

		rsCache.gsState().backup();
		rsCache.gsState().setShader(m_DrawGs);
		rsCache.gsState().setConstBuffers(0, 1, js::BufVA() << m_DrawCb);

		rsCache.psState().backup();
		rsCache.psState().setShader(m_DrawPs);

		rsCache.applyToContext(d3dContext);

		// draw
		d3dContext->DrawInstanced(1, SIZE, 0, 0);

		// restore shader resources
		rsCache.vsState().restore();
		rsCache.gsState().restore();
		rsCache.psState().restore();
	}

	D3DXVECTOR4 hdrParams() const
	{
		return m_HDRParams;
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

		void update(const CBaseCamera& camera, const D3DXMATRIX& worldMatrix)
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
		D3DXVECTOR4 m_HDRParams;

		void update(const CBaseCamera& camera, const D3DXVECTOR4& hdrParams)
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

			m_HDRParams = hdrParams;
		}
	};

	typedef js::ConstantBuffer_t<PSPostProcess> PSPostProcessConstBuf;

#pragma pack(pop)

// Global Variables
	CFirstPersonCamera m_Camera;
	RenderableMesh m_Mesh;
	D3DXMATRIX m_WorldMatrix;

	js::Texture2DRenderBuffer m_ColorBuffer[2];
	js::Texture2DRenderBuffer m_ColorBufferDnSamp4x[2];
	js::Texture2DRenderBuffer m_ColorBufferDnSamp16x;
	js::Texture2DRenderBuffer m_DepthBuffer;

	VSPreObjectConstBuf m_VsPreObjectConstBuf;
	PSPreObjectConstBuf m_PsPreObjectConstBuf;
	PSPostProcessConstBuf m_PSPostProcessConstBuf;
	js::SamplerState m_SceneSamplerState;
	js::SamplerState m_PostSamplerState;

	float m_ElapsedTime;
	Histogram m_Histogram;
	bool m_ShowGui;
	bool m_UseFullHistogram;
	bool m_UseToneMapping;
	bool m_UseDof;

	js::RenderStateCache m_RSCache;
	
	// post processing
	ScreenQuad m_ScreenQuad;
	js::VertexShader m_PostVtxShd;
	js::PixelShader m_PostDofShd;
	js::PixelShader m_PostCopyShd;
	js::PixelShader m_PostToneMapShd;
	js::PixelShader m_PostDnSamp4xShd;
	js::PixelShader m_PostBrPassShd;
	js::PixelShader m_PostBlurShd;

// Methods
	Example02()
		: m_ElapsedTime(0)
		, m_ShowGui(true)
		, m_UseFullHistogram(false)
		, m_UseToneMapping(true)
		, m_UseDof(true)
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
		m_SceneSamplerState.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SceneSamplerState.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SceneSamplerState.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SceneSamplerState.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		m_SceneSamplerState.create(d3dDevice);

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
		m_PostSamplerState.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_PostSamplerState.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_PostSamplerState.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		m_PostSamplerState.create(d3dDevice);
		m_PostVtxShd.createFromFile(d3dDevice, media(L"Example02/Post.Vtx.hlsl"), "Main");
		js_assert(m_PostVtxShd.valid());

		m_PostDofShd.createFromFile(d3dDevice, media(L"Example02/Post.DOF.hlsl"), "Main");
		js_assert(m_PostDofShd.valid());
		
		m_PostCopyShd.createFromFile(d3dDevice, media(L"Example02/Post.Copy.hlsl"), "Main");
		js_assert(m_PostCopyShd.valid());

		m_PostToneMapShd.createFromFile(d3dDevice, media(L"Example02/Post.ToneMapping.hlsl"), "Main");
		js_assert(m_PostToneMapShd.valid());

		m_PostDnSamp4xShd.createFromFile(d3dDevice, media(L"Example02/Post.DownSample4x.hlsl"), "Main");
		js_assert(m_PostDnSamp4xShd.valid());
		
		m_PostBrPassShd.createFromFile(d3dDevice, media(L"Example02/Post.BrightPass.hlsl"), "Main");
		js_assert(m_PostBrPassShd.valid());
		
		m_PostBlurShd.createFromFile(d3dDevice, media(L"Example02/Post.Blur.hlsl"), "Main");
		js_assert(m_PostBlurShd.valid());
		
		m_ScreenQuad.create(d3dDevice, m_PostVtxShd.m_ByteCode);
		js_assert(m_ScreenQuad.valid());

		m_PSPostProcessConstBuf.create(d3dDevice);

		D3DXVECTOR3 vecEye( 0.0f, m_Mesh.radius() * 0.5f, m_Mesh.radius() * -0.5f );
		D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
		m_Camera.SetViewParams( &vecEye, &vecAt );
		m_Camera.SetRotateButtons(TRUE, FALSE, FALSE);
		m_Camera.SetScalers( 0.01f, m_Mesh.radius() * 0.1f );
		m_Camera.SetDrag( true );
		m_Camera.SetEnableYAxisMovement( true );
		m_Camera.FrameMove(0);

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

		m_Camera.SetProjParams( D3DX_PI / 3, aspect, 0.1f, radius * 8.0f );
		// create render buffers
		
		for(int i=0; i<2; ++i)
		{
			m_ColorBuffer[i].create(d3dDevice, width, height, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
			js_assert(m_ColorBuffer[i].valid());

			m_ColorBufferDnSamp4x[i].create(d3dDevice, width / 4, height / 4, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
			js_assert(m_ColorBufferDnSamp4x[i].valid());
		}
			
		m_ColorBufferDnSamp16x.create(d3dDevice, width / 16, height / 16, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
		js_assert(m_ColorBufferDnSamp16x.valid());

		m_DepthBuffer.create(d3dDevice, width, height, 1,
			DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
		js_assert(m_DepthBuffer.valid());

		return S_OK;
	}

	__override void onD3D11ReleasingSwapChain()
	{
		guiOnD3D11ReleasingSwapChain();

		// destroy render buffers
		for(int i=0; i<2; ++i)
		{
			m_ColorBuffer[i].destroy();
			m_ColorBufferDnSamp4x[i].destroy();
		}
		m_ColorBufferDnSamp16x.destroy();
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
		m_PostDofShd.destroy();
		m_PostCopyShd.destroy();
		m_PostToneMapShd.destroy();
		m_PostDnSamp4xShd.destroy();
		m_PostBrPassShd.destroy();
		m_PostBlurShd.destroy();
		
		m_SceneSamplerState.destroy();
		m_PostSamplerState.destroy();
	}

	__override void onFrameMove(
		double time,
		float elapsedTime)
	{
		m_ElapsedTime = elapsedTime;
		m_Camera.FrameMove(elapsedTime);
		D3DXMatrixIdentity(&m_WorldMatrix);
	}

	__override void onD3D11FrameRender(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dImmediateContext,
		double time,
		float elapsedTime)
	{
		m_RSCache.rtState().set(1, js::RtvVA() << DXUTGetD3D11RenderTargetView(), DXUTGetD3D11DepthStencilView());

		m_RSCache.rtState().backup();
		m_RSCache.vsState().backup();
		m_RSCache.psState().backup();
		m_RSCache.gsState().backup();

		// Scene
		onD3D11FrameRender_ClearRenderTargets(d3dImmediateContext);
		m_RSCache.rtState().set(1, js::RtvVA() << m_ColorBuffer[0], m_DepthBuffer);

		onD3D11FrameRender_Scene_PrepareShaderResources(d3dImmediateContext);
		onD3D11FrameRender_Scene_DrawMesh(d3dImmediateContext);

		m_RSCache.vsState().restore();
		m_RSCache.psState().restore();
		m_RSCache.gsState().restore();
		m_RSCache.rtState().restore();

		// Post-Processing
		onD3D11FrameRender_PostProcessing(d3dImmediateContext);

		// Gui
		if(m_ShowGui)
		{
			m_GuiTxtHelper->Begin();
			m_GuiTxtHelper->SetInsertionPos( 2, 0 );
			m_GuiTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ) );
			m_GuiTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
			m_GuiTxtHelper->DrawTextLine( true == m_UseFullHistogram ? L"F2-Full Histogram [true]" : L"F2-Full Histogram [false]");
			m_GuiTxtHelper->DrawTextLine( true == m_UseToneMapping ? L"F3-Tone Mapping [true]" : L"F3-Tone Mapping [false]");
			m_GuiTxtHelper->End();
		}
	}

	void onD3D11FrameRender_ClearRenderTargets(ID3D11DeviceContext* d3dContext)
	{
		// clear render target and the depth stencil 
		//static const float clearColor[4] = { 2, 2, 2, 0 };
		static const float clearColor[4] = {0.8f, 0.8f, 2, 0 };

		d3dContext->ClearRenderTargetView( m_ColorBuffer[0].m_RTView, clearColor );
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
		m_RSCache.vsState().setConstBuffers(0, 1, js::BufVA() << m_VsPreObjectConstBuf);
		m_RSCache.psState().setConstBuffers(0, 1, js::BufVA() << m_PsPreObjectConstBuf);
		m_RSCache.psState().setSamplers(0, 1, js::SampVA() << m_SceneSamplerState);
	}
	
	void onD3D11FrameRender_Scene_DrawMesh(ID3D11DeviceContext* d3dContext)
	{
		m_RSCache.applyToContext(d3dContext);
		m_Mesh.render(d3dContext, &m_RSCache);
	}
	
	void onD3D11FrameRender_PostProcessing(ID3D11DeviceContext* d3dContext)
	{
		// depth stencil state
		m_RSCache.depthStencilState().backup();
		m_RSCache.depthStencilState().DepthEnable = FALSE;
		m_RSCache.depthStencilState().DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		m_RSCache.depthStencilState().dirty();

		if(m_UseToneMapping)
		{
			// down sampling
			onD3D11FrameRender_PostProcessing_Filter(
				d3dContext, m_ColorBuffer[0], m_ColorBufferDnSamp4x[0], m_PostDnSamp4xShd);
			onD3D11FrameRender_PostProcessing_Filter(
				d3dContext, m_ColorBufferDnSamp4x[0], m_ColorBufferDnSamp16x, m_PostDnSamp4xShd);

			// update histogram
			if(m_UseFullHistogram)
				m_Histogram.update(d3dContext, m_RSCache, m_ColorBuffer[0], m_ElapsedTime);
			else
				m_Histogram.update(d3dContext, m_RSCache, m_ColorBufferDnSamp16x, m_ElapsedTime);

			m_PSPostProcessConstBuf.map(d3dContext);
			m_PSPostProcessConstBuf.data().update(m_Camera, m_Histogram.hdrParams());
			m_PSPostProcessConstBuf.unmap(d3dContext);
			
			// brightpass
			onD3D11FrameRender_PostProcessing_Filter(
				d3dContext, m_ColorBuffer[0], m_ColorBufferDnSamp4x[0], m_PostBrPassShd);

			// blur the bloom
			onD3D11FrameRender_PostProcessing_Filter(
				d3dContext, m_ColorBufferDnSamp4x[0], m_ColorBufferDnSamp4x[1], m_PostBlurShd);

			onD3D11FrameRender_PostProcessing_Filter(
				d3dContext, m_ColorBufferDnSamp4x[1], m_ColorBufferDnSamp4x[0], m_PostBlurShd);
		}

		// depth of field
		if(m_UseDof)
		{
			m_RSCache.rtState().backup();
			m_RSCache.rtState().set(1, js::RtvVA() << m_ColorBuffer[1], nullptr);

			// shader states
			m_RSCache.vsState().backup();
			m_RSCache.vsState().setShader(m_PostVtxShd);

			m_RSCache.psState().backup();
			m_RSCache.psState().setShader(m_PostDofShd);
			m_RSCache.psState().setSRViews(0, 2, js::SrvVA() << m_ColorBuffer[0] << m_DepthBuffer);

			m_RSCache.psState().setConstBuffers(0, 1, js::BufVA() << m_PSPostProcessConstBuf);
			m_RSCache.psState().setSamplers(0, 1, js::SampVA() << m_PostSamplerState);

			m_RSCache.applyToContext(d3dContext);
			
			m_ScreenQuad.render(d3dContext);

			// restore shader states
			m_RSCache.rtState().restore();
			m_RSCache.vsState().restore();
			m_RSCache.psState().restore();
		}
		else
		{
			onD3D11FrameRender_PostProcessing_Filter(
				d3dContext, m_ColorBuffer[0], m_ColorBuffer[1], m_PostCopyShd);
		}

		// final output
		{
			// shader states
			m_RSCache.vsState().backup();
			m_RSCache.vsState().setShader(m_PostVtxShd);

			m_RSCache.psState().backup();
			if(m_UseToneMapping)
			{
				m_RSCache.psState().setShader(m_PostToneMapShd);
				m_RSCache.psState().setSRViews(0, 2, js::SrvVA() << m_ColorBuffer[1] << m_ColorBufferDnSamp4x[0]);
			}
			else
			{
				m_RSCache.psState().setShader(m_PostCopyShd);
				m_RSCache.psState().setSRViews(0, 1, js::SrvVA() << m_ColorBuffer[1]);
			}

			m_RSCache.psState().setConstBuffers(0, 1, js::BufVA() << m_PSPostProcessConstBuf);
			m_RSCache.psState().setSamplers(0, 1, js::SampVA() << m_PostSamplerState);

			m_RSCache.applyToContext(d3dContext);
			
			m_ScreenQuad.render(d3dContext);

			// restore shader states
			m_RSCache.vsState().restore();
			m_RSCache.psState().restore();
		}

		if(m_ShowGui)
			m_Histogram.display(d3dContext, m_RSCache, -0.9f, -0.5f, 0.5f, 0.25f);

		// restore depth stencil state
		m_RSCache.depthStencilState().restore();
	}
	
	void onD3D11FrameRender_PostProcessing_Filter(
		ID3D11DeviceContext* d3dContext,
		js::Texture2DRenderBuffer& srcBuf,
		js::Texture2DRenderBuffer& dstBuf,
		js::PixelShader& shader
		)
	{
		m_RSCache.rtState().backup();
		m_RSCache.rtState().set(1, js::RtvVA() << dstBuf, nullptr);

		m_RSCache.vsState().backup();
		m_RSCache.vsState().setShader(m_PostVtxShd);

		m_RSCache.psState().backup();
		m_RSCache.psState().setShader(shader);
		m_RSCache.psState().setSRViews(0, 1, js::SrvVA() << srcBuf);
		m_RSCache.psState().setConstBuffers(0, 1, js::BufVA() << m_PSPostProcessConstBuf);
		m_RSCache.psState().setSamplers(0, 1, js::SampVA() << m_PostSamplerState);

		m_RSCache.applyToContext(d3dContext);

		size_t vpCnt = 0; D3D11_VIEWPORT vps[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		d3dContext->RSGetViewports(&vpCnt, nullptr);
		d3dContext->RSGetViewports(&vpCnt, vps);
		d3dContext->RSSetViewports(1, js::VpVA() << dstBuf.viewport());

		m_ScreenQuad.render(d3dContext);

		d3dContext->RSSetViewports(vpCnt, vps);

		m_RSCache.vsState().restore();
		m_RSCache.psState().restore();
		m_RSCache.rtState().restore();
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
			m_ShowGui = !m_ShowGui;

		if(VK_F2 == nChar && keyDown)
			m_UseFullHistogram = !m_UseFullHistogram;

		if(VK_F3 == nChar && keyDown)
			m_UseToneMapping = !m_UseToneMapping;
		
		if(VK_F4 == nChar && keyDown)
			m_UseDof = !m_UseDof;
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


