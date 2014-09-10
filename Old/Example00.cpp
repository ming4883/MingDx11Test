#include "DXUT.h"

#include "SDKmisc.h"
#include "Common.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "Jessye/Shaders.h"
#include "Jessye/Buffers.h"
#include "Jessye/RenderStates.h"
#include "Jessye/ViewArrays.h"

class Example00 : public DXUTApp
{
public:
// Global Variables
	CFirstPersonCamera m_Camera;
	RenderableMesh m_Mesh;
	D3DXMATRIX m_WorldMatrix;

	js::Texture2DRenderBuffer m_ColorBuffer[2];
	js::Texture2DRenderBuffer m_ColorBufferDnSamp4x[2];
	js::Texture2DRenderBuffer m_DepthBuffer;

	SceneShaderConstantsBuffer m_SceneShdConstBuf;

	bool m_ShowGui;

	js::RenderStateCache m_RSCache;
	
	// post processing
	PostProcessor m_PostProcessor;
	js::PixelShader m_PostCopyShd;

	enum
	{
		UI_LIGHTCOLOR_R,
		UI_LIGHTCOLOR_G,
		UI_LIGHTCOLOR_B,
		UI_LIGHTCOLOR_MULTIPLER,
		UI_BGCOLOR_MULTIPLER,
	};

// Methods
	Example00()
		: m_ShowGui(true)
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

		m_GuiDlgs.push_back(dlg);
	}

	__override ~Example00()
	{
	}

	__override const wchar_t* getName() { return L"Example00"; }

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
		m_DepthBuffer.destroy();
	}

	__override void onD3D11DestroyDevice()
	{
		guiOnD3D11DestroyDevice();

		m_RSCache.destroy();

		DXUTGetGlobalResourceCache().OnDestroyDevice();
		m_Mesh.destroy();
		m_SceneShdConstBuf.destroy();

		m_PostProcessor.destroy();
		m_PostCopyShd.destroy();
	}

	__override void onFrameMove(
		double time,
		float elapsedTime)
	{
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
		// depth stencil state
		m_RSCache.depthStencilState().backup();
		m_RSCache.depthStencilState().DepthEnable = FALSE;
		m_RSCache.depthStencilState().DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		m_RSCache.depthStencilState().dirty();

		m_PostProcessor.m_ConstBuf.map(d3dContext);
		m_PostProcessor.m_ConstBuf.data().update(m_Camera);
		m_PostProcessor.m_ConstBuf.unmap(d3dContext);

		m_PostProcessor.filter(
			d3dContext, m_RSCache,
			js::SrvVA() << m_ColorBuffer[0],
			m_PostCopyShd);

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
	}

};	// Example00

// Initialize everything and go into a render loop
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	Example00 app;
	return DXUTApp::run(app);
}


