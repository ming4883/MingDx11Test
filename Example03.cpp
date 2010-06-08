#include "DXUT.h"

#include "SDKmisc.h"
#include "Common.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "Jessye/Shaders.h"
#include "Jessye/Buffers.h"
#include "Jessye/RenderStates.h"
#include "Jessye/ViewArrays.h"

class VolumeLightEffect
{
public:
	struct VolLightConstants
	{
#pragma pack(push)
#pragma pack(1)
		D3DXVECTOR4 m_VolSphere;	// x y z r
		D3DXVECTOR4 m_VolColor;
#pragma pack(pop)
	};

	struct Light
	{
		D3DXVECTOR4 m_Sphere;
		D3DXVECTOR4 m_Color;

		Light(
			float x, float y, float z, float radius,
			float r, float g, float b, float a)
			: m_Sphere(x, y, z, radius)
			, m_Color(r, g, b, a)
		{
		}
	};

	js::VertexShader m_VolLightVs;
	js::GeometryShader m_VolLightGs;
	js::PixelShader m_VolLightPs;
	js::ConstantBuffer_t<VolLightConstants> m_VolLightConstBuf;
	PointMesh m_PointMesh;

	VolumeLightEffect();
	~VolumeLightEffect();

	void create(ID3D11Device* d3dDevice);
	void destroy();
	bool valid() const;

	void render(
		ID3D11DeviceContext* d3dContext,
		js::RenderStateCache& rsCache,
		PostProcessor& postProcessor,
		SceneShaderConstantsBuffer& sceneShaderConstBuf,
		js::Texture2DRenderBuffer& depthBuffer,
		const Light& light);
};

VolumeLightEffect::VolumeLightEffect()
{
}

VolumeLightEffect::~VolumeLightEffect()
{
	destroy();
}

void VolumeLightEffect::create(ID3D11Device* d3dDevice)
{
	m_VolLightVs.createFromFile(d3dDevice, media(L"Example03/VolLight.Vs.hlsl"), "Main");
	js_assert(m_VolLightVs.valid());

	m_VolLightGs.createFromFile(d3dDevice, media(L"Example03/VolLight.Gs.hlsl"), "Main");
	js_assert(m_VolLightGs.valid());

	m_VolLightPs.createFromFile(d3dDevice, media(L"Example03/VolLight.Ps.hlsl"), "Main");
	js_assert(m_VolLightPs.valid());

	m_VolLightConstBuf.create(d3dDevice);
	js_assert(m_VolLightConstBuf.valid());

	m_PointMesh.create(d3dDevice, m_VolLightVs.m_ByteCode);
	js_assert(m_PointMesh.valid());
}

void VolumeLightEffect::destroy()
{
	m_VolLightVs.destroy();
	m_VolLightGs.destroy();
	m_VolLightPs.destroy();
	m_VolLightConstBuf.destroy();
	m_PointMesh.destroy();
}

bool VolumeLightEffect::valid() const
{
	return m_VolLightVs.valid()
		&& m_VolLightGs.valid()
		&& m_VolLightPs.valid()
		&& m_VolLightConstBuf.valid()
		&& m_PointMesh.valid();
}

void VolumeLightEffect::render(
	ID3D11DeviceContext* d3dContext,
	js::RenderStateCache& rsCache,
	PostProcessor& postProcessor,
	SceneShaderConstantsBuffer& sceneShaderConstBuf,
	js::Texture2DRenderBuffer& depthBuffer,
	const Light& light
	)
{
	// render state
	rsCache.depthStencilState().backup();
	rsCache.depthStencilState().DepthEnable = FALSE;
	rsCache.depthStencilState().DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	rsCache.depthStencilState().dirty();

	// shader state
	rsCache.vsState().backup();
	rsCache.vsState().setShader(m_VolLightVs);
	rsCache.vsState().setConstBuffers(0, 2, js::BufVA() << sceneShaderConstBuf << m_VolLightConstBuf);

	rsCache.gsState().backup();
	rsCache.gsState().setShader(m_VolLightGs);
	rsCache.gsState().setConstBuffers(0, 2, js::BufVA() << sceneShaderConstBuf << m_VolLightConstBuf);

	rsCache.psState().backup();
	rsCache.psState().setShader(m_VolLightPs);
	rsCache.psState().setConstBuffers(0, 3, js::BufVA() << sceneShaderConstBuf << m_VolLightConstBuf << postProcessor.m_ConstBuf);
	rsCache.psState().setSRViews(0, 1, js::SrvVA() << depthBuffer);

	rsCache.applyToContext(d3dContext);
	
	m_VolLightConstBuf.map(d3dContext);
	m_VolLightConstBuf.data().m_VolSphere = light.m_Sphere;
	m_VolLightConstBuf.data().m_VolColor = light.m_Color;
	m_VolLightConstBuf.unmap(d3dContext);

	// draw
	m_PointMesh.render(d3dContext, 1);

	rsCache.depthStencilState().restore();
	rsCache.vsState().restore();
	rsCache.gsState().restore();
	rsCache.psState().restore();
}

class Example03 : public DXUTApp
{
public:
// Global Variables
	CFirstPersonCamera m_Camera;
	RenderableMesh m_Mesh;
	D3DXMATRIX m_WorldMatrix;

	js::Texture2DRenderBuffer m_ColorBuffer[2];
	js::Texture2DRenderBuffer m_ColorBufferDnSamp2x[2];
	js::Texture2DRenderBuffer m_DepthBuffer;

	SceneShaderConstantsBuffer m_SceneShdConstBuf;

	bool m_ShowGui;

	js::RenderStateCache m_RSCache;
	
	// post processing
	PostProcessor m_PostProcessor;
	js::PixelShader m_PostCopyShd;
	js::PixelShader m_PostRadialBlurShd;
	VolumeLightEffect m_VolLightEffect;

	enum
	{
		UI_LIGHTCOLOR_R,
		UI_LIGHTCOLOR_G,
		UI_LIGHTCOLOR_B,
		UI_LIGHTCOLOR_MULTIPLER,
		UI_BGCOLOR_MULTIPLER,
		UI_VOLLIGHTCOLOR_MULTIPLER,
		UI_VOLLIGHT_SHAFT,
	};

// Methods
	Example03()
		: m_ShowGui(true)
	{
		CDXUTDialog* dlg = new CDXUTDialog;
		dlg->Init(m_GuiDlgResMgr);
		const int w = 120, h = 20;
		int y = 50;
		D3DCOLOR textClr = D3DCOLOR_ARGB(255, 204, 0, 0);

		dlg->AddStatic(UI_LIGHTCOLOR_R, L"Light.R", 0, y, w, h);
		dlg->GetStatic(UI_LIGHTCOLOR_R)->SetTextColor(textClr);
		dlg->AddSlider(UI_LIGHTCOLOR_R, w, y, w, h, 0, 255, 51);
		y += h;

		dlg->AddStatic(UI_LIGHTCOLOR_G, L"Light.G", 0, y, w, h);
		dlg->GetStatic(UI_LIGHTCOLOR_G)->SetTextColor(textClr);
		dlg->AddSlider(UI_LIGHTCOLOR_G, w, y, w, h, 0, 255, 53);
		y += h;

		dlg->AddStatic(UI_LIGHTCOLOR_B, L"Light.B", 0, y, w, h);
		dlg->GetStatic(UI_LIGHTCOLOR_B)->SetTextColor(textClr);
		dlg->AddSlider(UI_LIGHTCOLOR_B, w, y, w, h, 0, 255, 87);
		y += h;

		dlg->AddStatic(UI_LIGHTCOLOR_MULTIPLER, L"Light.Multipler", 0, y, w, h);
		dlg->GetStatic(UI_LIGHTCOLOR_MULTIPLER)->SetTextColor(textClr);
		dlg->AddSlider(UI_LIGHTCOLOR_MULTIPLER, w, y, w, h, 0, 1023, 225);
		y += h;

		dlg->AddStatic(UI_BGCOLOR_MULTIPLER, L"BgColor.Multipler", 0, y, w, h);
		dlg->GetStatic(UI_BGCOLOR_MULTIPLER)->SetTextColor(textClr);
		dlg->AddSlider(UI_BGCOLOR_MULTIPLER, w, y, w, h, 0, 511, 255);
		y += h;
		
		dlg->AddStatic(UI_VOLLIGHTCOLOR_MULTIPLER, L"VolLight.Multipler", 0, y, w, h);
		dlg->GetStatic(UI_VOLLIGHTCOLOR_MULTIPLER)->SetTextColor(textClr);
		dlg->AddSlider(UI_VOLLIGHTCOLOR_MULTIPLER, w, y, w, h, 0, 1023, 370);
		y += h;

		dlg->AddCheckBox(UI_VOLLIGHT_SHAFT, L"VolLight.LightShaft", 0, y, w, h, true);
		dlg->GetCheckBox(UI_VOLLIGHT_SHAFT)->SetTextColor(textClr);
		y += h;

		m_GuiDlgs.push_back(dlg);
	}

	__override ~Example03()
	{
	}

	__override const wchar_t* getName() { return L"Example03"; }

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

		// post processing
		m_PostProcessor.create(d3dDevice);
		js_assert(m_PostProcessor.valid());
		
		m_PostCopyShd.createFromFile(d3dDevice, media(L"Common/Shader/Post.Copy.hlsl"), "Main");
		js_assert(m_PostCopyShd.valid());

		m_PostRadialBlurShd.createFromFile(d3dDevice, media(L"Example03/Post.RadialBlur.hlsl"), "Main");
		js_assert(m_PostRadialBlurShd.valid());

		m_VolLightEffect.create(d3dDevice);
		js_assert(m_VolLightEffect.valid());
	
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

	__override void onD3D11DestroyDevice()
	{
		guiOnD3D11DestroyDevice();

		m_RSCache.destroy();

		DXUTGetGlobalResourceCache().OnDestroyDevice();
		m_Mesh.destroy();
		m_SceneShdConstBuf.destroy();

		m_VolLightEffect.destroy();
		m_PostProcessor.destroy();
		m_PostCopyShd.destroy();
		m_PostRadialBlurShd.destroy();
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

			m_ColorBufferDnSamp2x[i].create(d3dDevice, width / 2, height / 2, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
			js_assert(m_ColorBufferDnSamp2x[i].valid());
		}
		
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
			m_ColorBufferDnSamp2x[i].destroy();
		}
		m_DepthBuffer.destroy();
	}

	__override void onFrameMove(
		double time,
		float elapsedTime)
	{
		m_Camera.FrameMove(elapsedTime);
		D3DXMatrixIdentity(&m_WorldMatrix);

		guiUpdateStaticWithSlider(0, UI_LIGHTCOLOR_R, UI_LIGHTCOLOR_R, L"Light.R");
		guiUpdateStaticWithSlider(0, UI_LIGHTCOLOR_G, UI_LIGHTCOLOR_G, L"Light.G");
		guiUpdateStaticWithSlider(0, UI_LIGHTCOLOR_B, UI_LIGHTCOLOR_B, L"Light.B");
		guiUpdateStaticWithSlider(0, UI_LIGHTCOLOR_MULTIPLER, UI_LIGHTCOLOR_MULTIPLER, L"Light.Multipler");
		guiUpdateStaticWithSlider(0, UI_VOLLIGHTCOLOR_MULTIPLER, UI_VOLLIGHTCOLOR_MULTIPLER, L"VolLight.Multipler");
	}

	__override void onD3D11FrameRender(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dImmediateContext,
		double time,
		float elapsedTime)
	{
		m_RSCache.rtState().set(1, js::RtvVA() << DXUTGetD3D11RenderTargetView(), DXUTGetD3D11DepthStencilView());

		m_RSCache.rtState().backup();
		onD3D11FrameRender_ClearRenderTargets(d3dImmediateContext);
		m_RSCache.rtState().set(1, js::RtvVA() << m_ColorBuffer[0], m_DepthBuffer);

		// Scene
		m_RSCache.vsState().backup();
		m_RSCache.psState().backup();
		m_RSCache.gsState().backup();

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
			m_GuiTxtHelper->End();
		}
	}

	void onD3D11FrameRender_ClearRenderTargets(ID3D11DeviceContext* d3dContext)
	{
		// clear render target and the depth stencil 
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
		m_SceneShdConstBuf.data().m_AmbientColor = D3DXVECTOR4(0.2f, 0.24f, 1, 0.5f);
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
		m_PostProcessor.m_ConstBuf.map(d3dContext);
		m_PostProcessor.m_ConstBuf.data().update(m_Camera);
		m_PostProcessor.m_ConstBuf.unmap(d3dContext);
		
		// Scene image
		{
			m_RSCache.depthStencilState().backup();
			m_RSCache.depthStencilState().DepthEnable = FALSE;
			m_RSCache.depthStencilState().DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			m_RSCache.depthStencilState().dirty();
			
			m_PostProcessor.filter(
				d3dContext, m_RSCache,
				js::SrvVA() << m_ColorBuffer[0],
				m_PostCopyShd);
			
			m_RSCache.depthStencilState().restore();
		}

		float cost = cosf((float)::DXUTGetTime() * 0.25f);
		float sint = sinf((float)::DXUTGetTime() * 0.25f);

		D3DXVECTOR3 lightPos[2];
		lightPos[0] = D3DXVECTOR3(0,2.5f,4) + D3DXVECTOR3(cost,0,sint) * 4;
		lightPos[1] = D3DXVECTOR3(0,2.5f,4) + D3DXVECTOR3(-cost,0,-sint) * 4;

		D3DXVECTOR4 lightColor[2];
		float volLightMultipler = m_GuiDlgs[0]->GetSlider(UI_VOLLIGHTCOLOR_MULTIPLER)->GetValue() / 255.0f;
		lightColor[0] = D3DXVECTOR4(1, 0.6f, 0.6f, volLightMultipler);
		lightColor[1] = D3DXVECTOR4(0.6f, 0.6f, 1, volLightMultipler);

		float lightRadius = 1;

		const bool lightShaftEnabled = m_GuiDlgs[0]->GetCheckBox(UI_VOLLIGHT_SHAFT)->GetChecked();

		// Volume light effect
		for(size_t i=0; i<2; ++i)
		{
			VolumeLightEffect::Light light(
				lightPos[i].x, lightPos[i].y, lightPos[i].z, lightRadius,
				lightColor[i].x, lightColor[i].y, lightColor[i].z, lightColor[i].w
				);

			// light
			m_RSCache.rtState().backup();
			d3dContext->ClearRenderTargetView(m_ColorBuffer[1].m_RTView, D3DXVECTOR4(0,0,0,0));
			m_RSCache.rtState().set(1, js::RtvVA() << m_ColorBuffer[1], nullptr);

			m_VolLightEffect.render(
				d3dContext,
				m_RSCache,
				m_PostProcessor,
				m_SceneShdConstBuf,
				m_DepthBuffer,
				light);

			m_RSCache.rtState().restore();
			
			
			// update constants for radial blur
			m_PostProcessor.m_ConstBuf.map(d3dContext);
			m_PostProcessor.m_ConstBuf.data().update(m_Camera);

			D3DXVECTOR4 p(light.m_Sphere.x, light.m_Sphere.y, light.m_Sphere.z, 1);
			D3DXMATRIX viewProjectionMatrix = *m_Camera.GetViewMatrix() * *m_Camera.GetProjMatrix();
			D3DXVec4Transform(&p, &p, &viewProjectionMatrix);
			p.x /= p.w; p.y /= p.w;
			p.x *= 0.5f; p.y *= -0.5f;
			p.x += 0.5f; p.y += 0.5f;
			p.z = 1.0f;
			m_PostProcessor.m_ConstBuf.data().m_UserParams = p;

			m_PostProcessor.m_ConstBuf.unmap(d3dContext);

			// no depth test
			m_RSCache.depthStencilState().backup();
			m_RSCache.depthStencilState().DepthEnable = FALSE;
			m_RSCache.depthStencilState().DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			m_RSCache.depthStencilState().dirty();
			
			// radial blur
			if(lightShaftEnabled)
			{
				m_RSCache.rtState().backup();
				m_RSCache.rtState().set(1, js::RtvVA() << m_ColorBuffer[0], nullptr);
				
				m_PostProcessor.filter(
					d3dContext, m_RSCache,
					js::SrvVA() << m_ColorBuffer[1],
					m_PostRadialBlurShd);

				m_RSCache.rtState().restore();
			}

			// blend to target
			m_RSCache.blendState().backup();
			m_RSCache.blendState().RenderTarget[0].BlendEnable = TRUE;
			m_RSCache.blendState().RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			m_RSCache.blendState().RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			m_RSCache.blendState().RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			m_RSCache.blendState().RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			m_RSCache.blendState().RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			m_RSCache.blendState().RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			m_RSCache.blendState().RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			m_RSCache.blendState().dirty();
			
			if(lightShaftEnabled)
			{
				m_PostProcessor.filter(
					d3dContext, m_RSCache,
					js::SrvVA() << m_ColorBuffer[0],
					m_PostRadialBlurShd);
			}

			m_PostProcessor.filter(
				d3dContext, m_RSCache,
				js::SrvVA() << m_ColorBuffer[1],
				m_PostCopyShd);
			
			m_RSCache.blendState().restore();

			m_RSCache.depthStencilState().restore();
				
		}
		
		// restore depth stencil state
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

		if(VK_INSERT == nChar && keyDown)
			DXUTPause(!DXUTIsTimePaused(), false);
	}

};	// Example03

// Initialize everything and go into a render loop
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	Example03 app;
	return DXUTApp::run(app);
}


