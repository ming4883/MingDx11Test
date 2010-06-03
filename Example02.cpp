#include "DXUT.h"

#include "SDKmisc.h"
#include "Common.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
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
	struct ComputeConsts
	{
		D3DXVECTOR4 g_vInputParams;
	};

	struct DrawConsts
	{
		D3DXVECTOR4 g_vDrawParams;
	};

#pragma pack(pop)

	js::Texture2DRenderBuffer m_HistogramBuffer;
	js::Texture2DRenderBuffer m_HdrParamBuffer;
	js::VertexShader m_ComputeVs;
	js::GeometryShader m_ComputeGs;
	js::PixelShader m_ComputePs;
	js::VertexShader m_Compute2Vs;
	js::GeometryShader m_Compute2Gs;
	js::PixelShader m_Compute2Ps;
	js::VertexShader m_DrawVs;
	js::GeometryShader m_DrawGs;
	js::PixelShader m_DrawPs;
	js::PixelShader m_UpdatePs;
	js::ConstantBuffer_t<ComputeConsts> m_ComputeCb;
	js::ConstantBuffer_t<DrawConsts> m_DrawCb;
	PointMesh m_PointMesh;

	float m_LastNumInputs;
	float m_MaxInputValue;
	float m_AdaptFactor;
	float m_KeyTarget;
	float m_BloomThreshold;
	
	Histogram()
		: m_MaxInputValue(4.0f)
		, m_LastNumInputs(1)
		, m_AdaptFactor(2)
		, m_KeyTarget(0.95f)
		, m_BloomThreshold(0.5f)
	{
	}

	void create(ID3D11Device* d3dDevice)
	{
		m_HistogramBuffer.create(d3dDevice, SIZE, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		js_assert(m_HistogramBuffer.valid());
		
		m_HdrParamBuffer.create(d3dDevice, 1, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		js_assert(m_HdrParamBuffer.valid());

		{	float value[4] = {0, 1, 1, 0};
			DXUTGetD3D11DeviceContext()->ClearRenderTargetView(m_HdrParamBuffer, value);
		}

		m_ComputeVs.createFromFile(d3dDevice, media(L"Example02/Histogram.Compute.VS.hlsl"), "Main");
		js_assert(m_ComputeVs.valid());

		m_ComputeGs.createFromFile(d3dDevice, media(L"Example02/Histogram.Compute.GS.hlsl"), "Main");
		js_assert(m_ComputeGs.valid());

		m_ComputePs.createFromFile(d3dDevice, media(L"Example02/Histogram.Compute.PS.hlsl"), "Main");
		js_assert(m_ComputePs.valid());

		m_Compute2Vs.createFromFile(d3dDevice, media(L"Example02/Histogram.Compute2.VS.hlsl"), "Main");
		js_assert(m_Compute2Vs.valid());

		m_Compute2Gs.createFromFile(d3dDevice, media(L"Example02/Histogram.Compute2.GS.hlsl"), "Main");
		js_assert(m_Compute2Gs.valid());

		m_Compute2Ps.createFromFile(d3dDevice, media(L"Example02/Histogram.Compute2.PS.hlsl"), "Main");
		js_assert(m_Compute2Ps.valid());

		m_DrawVs.createFromFile(d3dDevice, media(L"Example02/Histogram.Draw.VS.hlsl"), "Main");
		js_assert(m_DrawVs.valid());

		m_DrawGs.createFromFile(d3dDevice, media(L"Example02/Histogram.Draw.GS.hlsl"), "Main");
		js_assert(m_DrawGs.valid());

		m_DrawPs.createFromFile(d3dDevice, media(L"Example02/Histogram.Draw.PS.hlsl"), "Main");
		js_assert(m_DrawPs.valid());

		m_UpdatePs.createFromFile(d3dDevice, media(L"Example02/Histogram.Update.PS.hlsl"), "Main");
		js_assert(m_UpdatePs.valid());

		m_ComputeCb.create(d3dDevice);
		js_assert(m_ComputeCb.valid());
		
		m_DrawCb.create(d3dDevice);
		js_assert(m_DrawCb.valid());

		m_PointMesh.create(d3dDevice, m_ComputeVs.m_ByteCode);
		js_assert(m_PointMesh.valid());
	}

	void destroy()
	{
		m_HistogramBuffer.destroy();
		m_HdrParamBuffer.destroy();
		m_ComputeVs.destroy();
		m_ComputeGs.destroy();
		m_ComputePs.destroy();
		m_Compute2Vs.destroy();
		m_Compute2Gs.destroy();
		m_Compute2Ps.destroy();
		m_DrawVs.destroy();
		m_DrawGs.destroy();
		m_DrawPs.destroy();
		m_UpdatePs.destroy();
		m_ComputeCb.destroy();
		m_DrawCb.destroy();
		m_PointMesh.destroy();
	}

	void compute(ID3D11DeviceContext* d3dContext, js::RenderStateCache& rsCache, js::Texture2DRenderBuffer& colorBuffer)
	{
		js_assert(colorBuffer.valid());

		m_ComputeCb.map(d3dContext);
		m_ComputeCb.data().g_vInputParams.x = (float)(colorBuffer.m_Width / 4);
		m_ComputeCb.data().g_vInputParams.y = 4;
		m_ComputeCb.data().g_vInputParams.z = m_MaxInputValue;
		m_ComputeCb.data().g_vInputParams.w = (float)SIZE;
		m_ComputeCb.unmap(d3dContext);
		
		// render target
		static const float clearColor[] = {0, 0, 0, 0};
		d3dContext->ClearRenderTargetView(m_HistogramBuffer.m_RTView, clearColor);
		size_t vpCnt = 0; D3D11_VIEWPORT vps[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		d3dContext->RSGetViewports(&vpCnt, nullptr);
		d3dContext->RSGetViewports(&vpCnt, vps);
		d3dContext->RSSetViewports(1, js::VpVA() << m_HistogramBuffer.viewport());

		rsCache.rtState().backup();
		rsCache.rtState().set(1, js::RtvVA() << m_HistogramBuffer, nullptr);

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
		m_PointMesh.render(d3dContext, (colorBuffer.m_Width * colorBuffer.m_Height) / 16);

		m_LastNumInputs = (float)(colorBuffer.m_Width * colorBuffer.m_Height);
		
		// restore render targets
		d3dContext->RSSetViewports(vpCnt, vps);

		rsCache.rtState().restore();
		rsCache.blendState().restore();
		rsCache.vsState().restore();
		rsCache.gsState().restore();
		rsCache.psState().restore();
	}

	void compute2(ID3D11DeviceContext* d3dContext, js::RenderStateCache& rsCache, js::Texture2DRenderBuffer& colorBuffer)
	{
		js_assert(colorBuffer.valid());

		const size_t iDiv = 8;

		m_ComputeCb.map(d3dContext);
		m_ComputeCb.data().g_vInputParams.x = (float)iDiv;
		m_ComputeCb.data().g_vInputParams.y = 0;
		m_ComputeCb.data().g_vInputParams.z = m_MaxInputValue;
		m_ComputeCb.data().g_vInputParams.w = (float)SIZE;
		m_ComputeCb.unmap(d3dContext);
		
		// render target
		static const float clearColor[] = {0, 0, 0, 0};
		d3dContext->ClearRenderTargetView(m_HistogramBuffer.m_RTView, clearColor);
		size_t vpCnt = 0; D3D11_VIEWPORT vps[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		d3dContext->RSGetViewports(&vpCnt, nullptr);
		d3dContext->RSGetViewports(&vpCnt, vps);
		d3dContext->RSSetViewports(1, js::VpVA() << m_HistogramBuffer.viewport());

		rsCache.rtState().backup();
		rsCache.rtState().set(1, js::RtvVA() << m_HistogramBuffer, nullptr);

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
		rsCache.vsState().setShader(m_Compute2Vs);
		rsCache.vsState().setConstBuffers(0, 1, js::BufVA() << m_ComputeCb);

		rsCache.gsState().backup();
		rsCache.gsState().setShader(m_Compute2Gs);
		rsCache.gsState().setSRViews(0, 1, js::SrvVA() << colorBuffer);
		rsCache.gsState().setConstBuffers(0, 1, js::BufVA() << m_ComputeCb);

		rsCache.psState().backup();
		rsCache.psState().setShader(m_Compute2Ps);
		rsCache.applyToContext(d3dContext);

		// draw
		m_PointMesh.render(d3dContext, iDiv * iDiv);

		m_LastNumInputs = (float)(((colorBuffer.m_Width / iDiv) * iDiv) * ((colorBuffer.m_Height / iDiv) * iDiv));
		
		// restore render targets
		d3dContext->RSSetViewports(vpCnt, vps);

		rsCache.rtState().restore();
		rsCache.blendState().restore();
		rsCache.vsState().restore();
		rsCache.gsState().restore();
		rsCache.psState().restore();
	}

	void update(ID3D11DeviceContext* d3dContext, js::RenderStateCache& rsCache, PostProcessor& postProcessor, float dt)
	{
		const float t = dt * m_AdaptFactor;

		postProcessor.m_ConstBuf.map(d3dContext);
		postProcessor.m_ConstBuf.data().m_UserParams.x = m_MaxInputValue;
		postProcessor.m_ConstBuf.data().m_UserParams.y = m_KeyTarget;
		postProcessor.m_ConstBuf.data().m_UserParams.z = m_BloomThreshold;
		postProcessor.m_ConstBuf.data().m_UserParams.w = (float)SIZE;
		postProcessor.m_ConstBuf.unmap(d3dContext);

		rsCache.blendState().backup();
		rsCache.blendState().RenderTarget[0].BlendEnable = TRUE;
		rsCache.blendState().RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		rsCache.blendState().RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rsCache.blendState().RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
		rsCache.blendState().RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
		rsCache.blendState().RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_BLEND_FACTOR;
		rsCache.blendState().RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_BLEND_FACTOR;
		rsCache.blendState().blendFactor()[0] = t;
		rsCache.blendState().blendFactor()[1] = t;
		rsCache.blendState().blendFactor()[2] = t;
		rsCache.blendState().blendFactor()[3] = t;
		rsCache.blendState().dirty();

		postProcessor.filter(
			d3dContext, rsCache, m_HistogramBuffer, m_HdrParamBuffer, m_UpdatePs);

		rsCache.blendState().restore();
	}

	void display(ID3D11DeviceContext* d3dContext, js::RenderStateCache& rsCache, float x, float y, float w, float h)
	{
		m_DrawCb.map(d3dContext);
		m_DrawCb.data().g_vDrawParams.x = x;
		m_DrawCb.data().g_vDrawParams.y = y;
		m_DrawCb.data().g_vDrawParams.z = w / SIZE;
		m_DrawCb.data().g_vDrawParams.w = h / m_LastNumInputs;
		m_DrawCb.unmap(d3dContext);

		//shader states
		rsCache.vsState().backup();
		rsCache.vsState().setShader(m_DrawVs);
		rsCache.vsState().setSRViews(0, 1, js::SrvVA() << m_HistogramBuffer);
		rsCache.vsState().setConstBuffers(0, 1, js::BufVA() << m_DrawCb);

		rsCache.gsState().backup();
		rsCache.gsState().setShader(m_DrawGs);
		rsCache.gsState().setConstBuffers(0, 1, js::BufVA() << m_DrawCb);

		rsCache.psState().backup();
		rsCache.psState().setShader(m_DrawPs);

		rsCache.applyToContext(d3dContext);

		// draw
		m_PointMesh.render(d3dContext, SIZE);

		// restore shader resources
		rsCache.vsState().restore();
		rsCache.gsState().restore();
		rsCache.psState().restore();
	}
};	// Histogram

class Example02 : public DXUTApp
{
public:
// Global Variables
	CFirstPersonCamera m_Camera;
	RenderableMesh m_Mesh;
	D3DXMATRIX m_WorldMatrix;

	js::Texture2DRenderBuffer m_ColorBuffer[2];
	js::Texture2DRenderBuffer m_ColorBufferDnSamp4x[2];
	js::Texture2DRenderBuffer m_ColorBufferDnSamp16x;
	js::Texture2DRenderBuffer m_DepthBuffer;
	js::Texture2DRenderBuffer m_DofFocusBuffer;

	SceneShaderConstantsBuffer m_SceneShdConstBuf;

	Histogram m_Histogram;
	bool m_ShowGui;
	bool m_UseFullHistogram;
	int m_HistogramMethod;

	js::RenderStateCache m_RSCache;
	
	// post processing
	PostProcessor m_PostProcessor;
	js::PixelShader m_PostDofShd;
	js::PixelShader m_PostDofFocusShd;
	js::PixelShader m_PostCopyShd;
	js::PixelShader m_PostToneMapShd;
	js::PixelShader m_PostDnSamp4xShd;
	js::PixelShader m_PostBrPassShd;
	js::PixelShader m_PostBlurShd;

	enum
	{
		UI_LIGHTCOLOR_R,
		UI_LIGHTCOLOR_G,
		UI_LIGHTCOLOR_B,
		UI_LIGHTCOLOR_MULTIPLER,
		UI_BGCOLOR_MULTIPLER,
		UI_POST_ADAPTTIME,
		UI_POST_HDR_ENABLE,
		UI_POST_HDR_BLOOMTHRESHOLD,
		UI_POST_HDR_WHITETARGET,
		UI_POST_DOF_ENABLE,
		UI_POST_DOF_BEGIN,
		UI_POST_DOF_RANGE,
	};

// Methods
	Example02()
		: m_ShowGui(true)
		, m_UseFullHistogram(false)
		, m_HistogramMethod(0)
	{
		CDXUTDialog* dlg = new CDXUTDialog;
		dlg->Init(m_GuiDlgResMgr);
		const int w = 120, h = 20;
		int y = 50;
		D3DCOLOR textClr = D3DCOLOR_ARGB(255, 204, 0, 0);

		dlg->AddStatic(UI_LIGHTCOLOR_R, L"Light.R", 0, y, w, h);
		dlg->GetStatic(UI_LIGHTCOLOR_R)->SetTextColor(textClr);
		dlg->AddSlider(UI_LIGHTCOLOR_R, w, y, w, h, 0, 255, 255);
		y += h;

		dlg->AddStatic(UI_LIGHTCOLOR_G, L"Light.G", 0, y, w, h);
		dlg->GetStatic(UI_LIGHTCOLOR_G)->SetTextColor(textClr);
		dlg->AddSlider(UI_LIGHTCOLOR_G, w, y, w, h, 0, 255, 202);
		y += h;

		dlg->AddStatic(UI_LIGHTCOLOR_B, L"Light.B", 0, y, w, h);
		dlg->GetStatic(UI_LIGHTCOLOR_B)->SetTextColor(textClr);
		dlg->AddSlider(UI_LIGHTCOLOR_B, w, y, w, h, 0, 255, 87);
		y += h;

		dlg->AddStatic(UI_LIGHTCOLOR_MULTIPLER, L"Light.Multipler", 0, y, w, h);
		dlg->GetStatic(UI_LIGHTCOLOR_MULTIPLER)->SetTextColor(textClr);
		dlg->AddSlider(UI_LIGHTCOLOR_MULTIPLER, w, y, w, h, 0, 1023, 511);
		y += h;

		dlg->AddStatic(UI_BGCOLOR_MULTIPLER, L"BgColor.Multipler", 0, y, w, h);
		dlg->GetStatic(UI_BGCOLOR_MULTIPLER)->SetTextColor(textClr);
		dlg->AddSlider(UI_BGCOLOR_MULTIPLER, w, y, w, h, 0, 511, 255);
		y += h;

		dlg->AddStatic(UI_POST_ADAPTTIME, L"Post.AdaptTime", 0, y, w, h);
		dlg->GetStatic(UI_POST_ADAPTTIME)->SetTextColor(textClr);
		dlg->AddSlider(UI_POST_ADAPTTIME, w, y, w, h, 0, 1023, 127);
		y += h;
		
		dlg->AddCheckBox(UI_POST_HDR_ENABLE, L"Post.HDR.Enable", 0, y, w, h, true);
		dlg->GetCheckBox(UI_POST_HDR_ENABLE)->SetTextColor(textClr);
		y += h;

		dlg->AddStatic(UI_POST_HDR_BLOOMTHRESHOLD, L"Post.HDR.BloomThreshold", 0, y, w, h);
		dlg->GetStatic(UI_POST_HDR_BLOOMTHRESHOLD)->SetTextColor(textClr);
		dlg->AddSlider(UI_POST_HDR_BLOOMTHRESHOLD, w, y, w, h, 0, 255, 127);
		y += h;

		dlg->AddStatic(UI_POST_HDR_WHITETARGET, L"Post.HDR.KeyTarget", 0, y, w, h);
		dlg->GetStatic(UI_POST_HDR_WHITETARGET)->SetTextColor(textClr);
		dlg->AddSlider(UI_POST_HDR_WHITETARGET, w, y, w, h, 0, 255, (int)(255 * 0.8f));
		y += h;

		dlg->AddCheckBox(UI_POST_DOF_ENABLE, L"Post.DOF.Enable", 0, y, w, h, true);
		dlg->GetCheckBox(UI_POST_DOF_ENABLE)->SetTextColor(textClr);
		y += h;

		dlg->AddStatic(UI_POST_DOF_BEGIN, L"Post.DOF.Begin", 0, y, w, h);
		dlg->GetStatic(UI_POST_DOF_BEGIN)->SetTextColor(textClr);
		dlg->AddSlider(UI_POST_DOF_BEGIN, w, y, w, h, 1, 16, 1);
		y += h;

		dlg->AddStatic(UI_POST_DOF_RANGE, L"Post.DOF.Range", 0, y, w, h);
		dlg->GetStatic(UI_POST_DOF_RANGE)->SetTextColor(textClr);
		dlg->AddSlider(UI_POST_DOF_RANGE, w, y, w, h, 1, 16, 3);
		y += h;

		m_GuiDlgs.push_back(dlg);
	}

	__override ~Example02()
	{
	}

	__override const wchar_t* getName() { return L"Example02"; }

	__override bool isD3D11DeviceAcceptable(
		const CD3D11EnumAdapterInfo* adapterInfo,
		UINT output,
		const CD3D11EnumDeviceInfo* deviceInfo,
		DXGI_FORMAT backBufferFormat,
		bool windowed)
	{
		//if(FALSE == deviceInfo->ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
		//	return false;

		return true;
	}

	__override HRESULT onD3D11CreateDevice(
		ID3D11Device* d3dDevice,
		const DXGI_SURFACE_DESC* backBufferSurfaceDesc)
	{
		guiOnD3D11CreateDevice(d3dDevice);

		m_RSCache.create(d3dDevice);

		// scene
		RenderableMesh::ShaderDesc sd;
		sd.vsPath = media(L"Common/Shader/Scene.VS.hlsl");
		sd.vsEntry = "VSMain";

		sd.psPath = media(L"Common/Shader/Scene.PS.hlsl");
		sd.psEntry = "PSMain";

		std::vector<D3D11_INPUT_ELEMENT_DESC> ielems;
		inputElement(ielems, "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0);
		inputElement(ielems, "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0);
		inputElement(ielems, "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0);

		m_Mesh.create(d3dDevice, media(L"Common/SoftParticles/TankScene.sdkmesh"), sd, ielems);

		m_SceneShdConstBuf.create(d3dDevice);

		m_Histogram.create(d3dDevice);

		// post processing
		m_PostProcessor.create(d3dDevice);
		js_assert(m_PostProcessor.valid());
		
		m_PostDofShd.createFromFile(d3dDevice, media(L"Example02/Post.DOF.hlsl"), "Main");
		js_assert(m_PostDofShd.valid());
		
		m_PostDofFocusShd.createFromFile(d3dDevice, media(L"Example02/Post.DOF.Focus.hlsl"), "Main");
		js_assert(m_PostDofFocusShd.valid());
		
		m_PostCopyShd.createFromFile(d3dDevice, media(L"Common/Shader/Post.Copy.hlsl"), "Main");
		js_assert(m_PostCopyShd.valid());

		m_PostToneMapShd.createFromFile(d3dDevice, media(L"Example02/Post.ToneMapping.hlsl"), "Main");
		js_assert(m_PostToneMapShd.valid());

		m_PostDnSamp4xShd.createFromFile(d3dDevice, media(L"Example02/Post.DownSample4x.hlsl"), "Main");
		js_assert(m_PostDnSamp4xShd.valid());
		
		m_PostBrPassShd.createFromFile(d3dDevice, media(L"Example02/Post.BrightPass.hlsl"), "Main");
		js_assert(m_PostBrPassShd.valid());
		
		m_PostBlurShd.createFromFile(d3dDevice, media(L"Example02/Post.Blur.hlsl"), "Main");
		js_assert(m_PostBlurShd.valid());
		
		// init camera
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

		m_GuiDlgs[0]->SetLocation(backBufferSurfaceDesc->Width-260, 0);
		m_GuiDlgs[0]->SetSize(260, backBufferSurfaceDesc->Height);

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

		m_DofFocusBuffer.create(d3dDevice, 1, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		js_assert(m_DofFocusBuffer.valid());

		{	float value[4] = {0, 0, 0, 0};
			DXUTGetD3D11DeviceContext()->ClearRenderTargetView(m_DofFocusBuffer, value);
		}

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
		m_DofFocusBuffer.destroy();
	}

	__override void onD3D11DestroyDevice()
	{
		guiOnD3D11DestroyDevice();

		m_RSCache.destroy();

		DXUTGetGlobalResourceCache().OnDestroyDevice();
		m_Mesh.destroy();
		m_SceneShdConstBuf.destroy();

		m_Histogram.destroy();

		m_PostProcessor.destroy();
		m_PostDofShd.destroy();
		m_PostDofFocusShd.destroy();
		m_PostCopyShd.destroy();
		m_PostToneMapShd.destroy();
		m_PostDnSamp4xShd.destroy();
		m_PostBrPassShd.destroy();
		m_PostBlurShd.destroy();
	}

	__override void onFrameMove(
		double time,
		float elapsedTime)
	{
		m_Camera.FrameMove(elapsedTime);
		D3DXMatrixIdentity(&m_WorldMatrix);

		// adapt-factor = 1 / adapt-time
		m_Histogram.m_AdaptFactor = 256.0f / (m_GuiDlgs[0]->GetSlider(UI_POST_ADAPTTIME)->GetValue()+1);
		m_Histogram.m_BloomThreshold = m_GuiDlgs[0]->GetSlider(UI_POST_HDR_BLOOMTHRESHOLD)->GetValue() / 255.0f;
		m_Histogram.m_KeyTarget = m_GuiDlgs[0]->GetSlider(UI_POST_HDR_WHITETARGET)->GetValue() / 255.0f;
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
		onD3D11FrameRender_PostProcessing(d3dImmediateContext, elapsedTime);

		// Gui
		if(m_ShowGui)
		{
			m_GuiDlgs[0]->OnRender(elapsedTime);
			m_GuiTxtHelper->Begin();
			m_GuiTxtHelper->SetInsertionPos( 2, 0 );
			m_GuiTxtHelper->SetForegroundColor( D3DXCOLOR( 0.8f, 0.0f, 0.0f, 1.0f ) );
			m_GuiTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
			m_GuiTxtHelper->DrawTextLine( true == m_UseFullHistogram ? L"F2-Full Histogram [true]" : L"F2-Full Histogram [false]");
			m_GuiTxtHelper->DrawTextLine( 0 == m_HistogramMethod ? L"F3-HistogramMethod [0]" : L"F3-HistogramMethod [1]");
			m_GuiTxtHelper->End();
		}
	}

	void onD3D11FrameRender_ClearRenderTargets(ID3D11DeviceContext* d3dContext)
	{
		// clear render target and the depth stencil 
		//static const float clearColor[4] = { 2, 2, 2, 0 };
		static float clearColor[4] = {0.8f, 0.8f, 2, 0 };
		clearColor[0] = m_GuiDlgs[0]->GetSlider(UI_LIGHTCOLOR_R)->GetValue() / 255.0f;
		clearColor[1] = m_GuiDlgs[0]->GetSlider(UI_LIGHTCOLOR_G)->GetValue() / 255.0f;
		clearColor[2] = m_GuiDlgs[0]->GetSlider(UI_LIGHTCOLOR_B)->GetValue() / 255.0f;
		clearColor[3] = m_GuiDlgs[0]->GetSlider(UI_LIGHTCOLOR_MULTIPLER)->GetValue() / 255.0f;
		clearColor[3] *= m_GuiDlgs[0]->GetSlider(UI_BGCOLOR_MULTIPLER)->GetValue() / 255.0f;

		clearColor[0] *= clearColor[3];
		clearColor[1] *= clearColor[3];
		clearColor[2] *= clearColor[3];
		clearColor[3] = 0;

		d3dContext->ClearRenderTargetView( m_ColorBuffer[0].m_RTView, clearColor );
		d3dContext->ClearDepthStencilView( m_DepthBuffer.m_DSView, D3D11_CLEAR_DEPTH, 1.0, 0 );
	}

	void onD3D11FrameRender_Scene_PrepareShaderResources(ID3D11DeviceContext* d3dContext)
	{
		// m_SceneShdConstBuf
		m_SceneShdConstBuf.map(d3dContext);
		D3DXMatrixTranspose(&m_SceneShdConstBuf.data().m_World, &m_WorldMatrix);
		m_SceneShdConstBuf.data().updateCameraContants(m_Camera);
		m_SceneShdConstBuf.data().m_AmbientColor = D3DXVECTOR4(0.2f, 0.24f, 1, 1);
		m_SceneShdConstBuf.data().m_LightColor.x = m_GuiDlgs[0]->GetSlider(UI_LIGHTCOLOR_R)->GetValue() / 255.0f;
		m_SceneShdConstBuf.data().m_LightColor.y = m_GuiDlgs[0]->GetSlider(UI_LIGHTCOLOR_G)->GetValue() / 255.0f;
		m_SceneShdConstBuf.data().m_LightColor.z = m_GuiDlgs[0]->GetSlider(UI_LIGHTCOLOR_B)->GetValue() / 255.0f;
		m_SceneShdConstBuf.data().m_LightColor.w = m_GuiDlgs[0]->GetSlider(UI_LIGHTCOLOR_MULTIPLER)->GetValue() / 255.0f;
		m_SceneShdConstBuf.data().m_LightVector = D3DXVECTOR4(0, 1, 0, 0);
		m_SceneShdConstBuf.unmap(d3dContext);

		// preparing shaders
		m_RSCache.vsState().setConstBuffers(0, 1, js::BufVA() << m_SceneShdConstBuf);
		m_RSCache.psState().setConstBuffers(0, 1, js::BufVA() << m_SceneShdConstBuf);

		m_RSCache.samplerState().AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		m_RSCache.samplerState().AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		m_RSCache.samplerState().AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		m_RSCache.samplerState().Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		m_RSCache.samplerState().dirty();
		
		m_RSCache.psState().setSamplers(0, 1, js::SampVA() << *m_RSCache.samplerState().current());
	}
	
	void onD3D11FrameRender_Scene_DrawMesh(ID3D11DeviceContext* d3dContext)
	{
		m_RSCache.applyToContext(d3dContext);
		m_Mesh.render(d3dContext, &m_RSCache);
	}
	
	void onD3D11FrameRender_PostProcessing(ID3D11DeviceContext* d3dContext, float elapsedTime)
	{
		const bool cUseToneMapping = m_GuiDlgs[0]->GetCheckBox(UI_POST_HDR_ENABLE)->GetChecked();
		const bool cUseDOF = m_GuiDlgs[0]->GetCheckBox(UI_POST_DOF_ENABLE)->GetChecked();

		// depth stencil state
		m_RSCache.depthStencilState().backup();
		m_RSCache.depthStencilState().DepthEnable = FALSE;
		m_RSCache.depthStencilState().DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		m_RSCache.depthStencilState().dirty();

		m_PostProcessor.m_ConstBuf.map(d3dContext);
		m_PostProcessor.m_ConstBuf.data().update(m_Camera);
		m_PostProcessor.m_ConstBuf.unmap(d3dContext);

		if(cUseToneMapping)
		{
			// down sampling
			m_PostProcessor.filter(
				d3dContext, m_RSCache, m_ColorBuffer[0], m_ColorBufferDnSamp4x[0], m_PostDnSamp4xShd);
			//m_PostProcessor.filter(
			//	d3dContext, m_RSCache, m_ColorBufferDnSamp4x[0], m_ColorBufferDnSamp16x, m_PostDnSamp4xShd);

			// update histogram
			if(0 == m_HistogramMethod)
			{
				m_Histogram.compute(d3dContext, m_RSCache, m_UseFullHistogram ? m_ColorBuffer[0] : m_ColorBufferDnSamp4x[0]);
			}
			else
			{
				m_Histogram.compute2(d3dContext, m_RSCache, m_UseFullHistogram ? m_ColorBuffer[0] : m_ColorBufferDnSamp4x[0]);
			}

			m_Histogram.update(d3dContext, m_RSCache, m_PostProcessor, elapsedTime);

			// brightpass
			m_PostProcessor.filter(
				d3dContext, m_RSCache,
				js::SrvVA() << m_ColorBuffer[0] << m_Histogram.m_HdrParamBuffer,
				m_ColorBufferDnSamp4x[0], m_PostBrPassShd);

			// blur the bloom
			m_PostProcessor.filter(
				d3dContext, m_RSCache, m_ColorBufferDnSamp4x[0], m_ColorBufferDnSamp4x[1], m_PostBlurShd);

			m_PostProcessor.filter(
				d3dContext, m_RSCache, m_ColorBufferDnSamp4x[1], m_ColorBufferDnSamp4x[0], m_PostBlurShd);
		}

		{
			const float t = elapsedTime * m_Histogram.m_AdaptFactor * 0.5f;

			m_RSCache.blendState().backup();
			m_RSCache.blendState().RenderTarget[0].BlendEnable = TRUE;
			m_RSCache.blendState().RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			m_RSCache.blendState().RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			m_RSCache.blendState().RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
			m_RSCache.blendState().RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
			m_RSCache.blendState().RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_BLEND_FACTOR;
			m_RSCache.blendState().RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_BLEND_FACTOR;
			m_RSCache.blendState().blendFactor()[0] = t;
			m_RSCache.blendState().blendFactor()[1] = t;
			m_RSCache.blendState().blendFactor()[2] = t;
			m_RSCache.blendState().blendFactor()[3] = t;
			m_RSCache.blendState().dirty();

			// measure dof focus
			m_PostProcessor.filter(
				d3dContext, m_RSCache, m_DepthBuffer, m_DofFocusBuffer, m_PostDofFocusShd);

			m_RSCache.blendState().restore();
		}

		// depth of field
		if(cUseDOF)
		{
			float dofBegin = (float)m_GuiDlgs[0]->GetSlider(UI_POST_DOF_BEGIN)->GetValue();
			float dofRange = (float)m_GuiDlgs[0]->GetSlider(UI_POST_DOF_RANGE)->GetValue();

			m_PostProcessor.m_ConstBuf.map(d3dContext);
			m_PostProcessor.m_ConstBuf.data().m_UserParams.x = dofBegin;
			m_PostProcessor.m_ConstBuf.data().m_UserParams.y = dofBegin + dofRange;
			m_PostProcessor.m_ConstBuf.data().m_UserParams.z = 3;
			m_PostProcessor.m_ConstBuf.data().m_UserParams.w = 0;
			m_PostProcessor.m_ConstBuf.unmap(d3dContext);

			m_PostProcessor.filter(
				d3dContext, m_RSCache,
				js::SrvVA() << m_ColorBuffer[0] << m_DepthBuffer << m_DofFocusBuffer,
				m_ColorBuffer[1], m_PostDofShd);
		}
		else
		{
			m_PostProcessor.filter(
				d3dContext, m_RSCache, m_ColorBuffer[0], m_ColorBuffer[1], m_PostCopyShd);
		}

		// final output
		if(cUseToneMapping)
		{
			m_PostProcessor.filter(
				d3dContext, m_RSCache,
				js::SrvVA() << m_ColorBuffer[1] << m_ColorBufferDnSamp4x[0] << m_Histogram.m_HdrParamBuffer,
				m_PostToneMapShd);
		}
		else
		{
			m_PostProcessor.filter(
				d3dContext, m_RSCache,
				js::SrvVA() << m_ColorBuffer[1],
				m_PostCopyShd);
		}

		if(m_ShowGui)
			m_Histogram.display(d3dContext, m_RSCache, -0.9f, -0.5f, 0.5f, 0.25f);

		// restore depth stencil state
		m_RSCache.depthStencilState().restore();
	}

	__override LRESULT msgProc(
		HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing)
	{
		this->guiMsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing);

		if(*pbNoFurtherProcessing)
			return 0;

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
			m_HistogramMethod = 1 - m_HistogramMethod;
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


