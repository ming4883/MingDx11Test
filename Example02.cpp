#include "DXUT.h"

#include "SDKmisc.h"
#include "Common.h"
#include "DXUTcamera.h"
#include "Jessye/Shaders.h"
#include "Jessye/Buffers.h"
#include "Jessye/RenderStates.h"

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
//#pragma pack(show)

	class Rendering
	{
	public:
		js::Texture2DRenderBuffer m_ColorBuffer;
		js::Texture2DRenderBuffer m_ColorBufferQuater;
		js::Texture2DRenderBuffer m_DepthBuffer;

		void onSwapChainResized(ID3D11Device* d3dDevice, size_t width, size_t height)
		{
			m_ColorBuffer.create(d3dDevice, width, height, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
			js_assert(m_ColorBuffer.valid());
			
			m_ColorBufferQuater.create(d3dDevice, width / 4, height / 4, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
			js_assert(m_ColorBufferQuater.valid());

			m_DepthBuffer.create(d3dDevice, width, height, 1,
				DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
			js_assert(m_DepthBuffer.valid());
		}

		void onSwapChainReleasing()
		{
			m_ColorBuffer.destroy();
			m_ColorBufferQuater.destroy();
			m_DepthBuffer.destroy();
		}

		void prepareRenderScene(
			ID3D11Device* d3dDevice,
			ID3D11DeviceContext* d3dContext)
		{
			// Clear render target and the depth stencil 
			//static const float clearColor[4] = { 0.176f, 0.176f, 0.176f, 0.0f };
			static const float clearColor[4] = { 2, 2, 2, 0 };

			d3dContext->OMSetRenderTargets(1, &m_ColorBuffer.m_RTView, m_DepthBuffer.m_DSView);
			d3dContext->ClearRenderTargetView( m_ColorBuffer.m_RTView, clearColor );
			d3dContext->ClearDepthStencilView( m_DepthBuffer.m_DSView, D3D11_CLEAR_DEPTH, 1.0, 0 );
		}

		void unprepareRenderScene(
			ID3D11Device* d3dDevice,
			ID3D11DeviceContext* d3dContext)
		{
			ID3D11RenderTargetView* rtv[] = {DXUTGetD3D11RenderTargetView()};
			d3dContext->OMSetRenderTargets(1, rtv, DXUTGetD3D11DepthStencilView());
		}
	};

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
		js::BlendState m_BlendState;
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

			m_BlendState.RenderTarget[0].BlendEnable = TRUE;
			m_BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			m_BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			m_BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			m_BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			m_BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			m_BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			m_BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			m_BlendState.AlphaToCoverageEnable = FALSE;
			m_BlendState.IndependentBlendEnable = FALSE;
			m_BlendState.create(d3dDevice);
			js_assert(m_BlendState.valid());
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
			m_BlendState.destroy();
		}

		void update(ID3D11DeviceContext* d3dContext, js::Texture2DRenderBuffer& colorBuffer)
		{
			js_assert(colorBuffer.valid());

			// render target
			static const float clearColor[] = {0, 0, 0, 0};
			d3dContext->ClearRenderTargetView(m_Buffer.m_RTView, clearColor);
			d3dContext->OMSetRenderTargets(1, &m_Buffer.m_RTView, nullptr);
			D3D11_VIEWPORT vp = m_Buffer.viewport();
			d3dContext->RSSetViewports(1, &vp);

			// shaders
			m_ComputeCb.map(d3dContext);
			m_ComputeCb.data().g_vInputParams.x = (float)(colorBuffer.m_Width / 4);
			m_ComputeCb.data().g_vInputParams.y = 4;
			m_ComputeCb.data().g_vInputParams.z = m_MaxInputValue;
			m_ComputeCb.unmap(d3dContext);

			d3dContext->VSSetShader(m_ComputeVs.m_ShaderObject, nullptr, 0);
			
			d3dContext->GSSetShader(m_ComputeGs.m_ShaderObject, nullptr, 0);
			d3dContext->GSSetShaderResources(0, 1, &colorBuffer.m_SRView);
			d3dContext->GSSetConstantBuffers(0, 1, &m_ComputeCb.m_BufferObject);

			d3dContext->PSSetShader(m_ComputePs.m_ShaderObject, nullptr, 0);

			// vertex buffer and input assembler
			UINT strides[] = {sizeof(D3DXVECTOR3)};
			UINT offsets[] = {0};

			d3dContext->IASetInputLayout(m_IL);
			d3dContext->IASetVertexBuffers(0, 1, &m_VB, strides, offsets);
			d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

			// render state
			d3dContext->OMSetBlendState(m_BlendState, nullptr, 0xffffffff);
			
			// draw
			d3dContext->DrawInstanced(1, (colorBuffer.m_Width * colorBuffer.m_Height) / 16, 0, 0);

			m_LastNumInputs = (float)(colorBuffer.m_Width * colorBuffer.m_Height);
			
			// restore render targets
			ID3D11RenderTargetView* rtv[] = {DXUTGetD3D11RenderTargetView()};
			d3dContext->OMSetRenderTargets(1, rtv, DXUTGetD3D11DepthStencilView());

			vp.Width = (float)DXUTGetDXGIBackBufferSurfaceDesc()->Width;
			vp.Height = (float)DXUTGetDXGIBackBufferSurfaceDesc()->Height;
			vp.TopLeftX = 0; vp.TopLeftY = 0; vp.MinDepth = 0; vp.MaxDepth = 1;
			
			d3dContext->RSSetViewports(1, &vp);

			{	// reset PS shader resources
				ID3D11ShaderResourceView* shv[] = {nullptr};
				d3dContext->VSSetShaderResources(0, 1, shv);
			}

			d3dContext->GSSetShader(nullptr, nullptr, 0);

			d3dContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		}

		void display(ID3D11DeviceContext* d3dContext, float x, float y, float w, float h)
		{
			// shaders
			m_DrawCb.map(d3dContext);
			m_DrawCb.data().g_vDrawParams.x = x;
			m_DrawCb.data().g_vDrawParams.y = y;
			m_DrawCb.data().g_vDrawParams.z = w / SIZE;
			m_DrawCb.data().g_vDrawParams.w = h / m_LastNumInputs;
			m_DrawCb.unmap(d3dContext);

			d3dContext->VSSetShader(m_DrawVs.m_ShaderObject, nullptr, 0);
			d3dContext->VSSetShaderResources(0, 1, &m_Buffer.m_SRView);
			d3dContext->VSSetConstantBuffers(0, 1, &m_DrawCb.m_BufferObject);

			d3dContext->GSSetShader(m_DrawGs.m_ShaderObject, nullptr, 0);
			d3dContext->GSSetConstantBuffers(0, 1, &m_DrawCb.m_BufferObject);

			d3dContext->PSSetShader(m_DrawPs.m_ShaderObject, nullptr, 0);

			// vertex buffer and input assembler
			UINT strides[] = {sizeof(D3DXVECTOR3)};
			UINT offsets[] = {0};

			d3dContext->IASetInputLayout(m_IL);
			d3dContext->IASetVertexBuffers(0, 1, &m_VB, strides, offsets);
			d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			
			// draw
			d3dContext->DrawInstanced(1, SIZE, 0, 0);

			{	// reset PS shader resources
				ID3D11ShaderResourceView* shv[] = {nullptr};
				d3dContext->VSSetShaderResources(0, 1, shv);
			}
			d3dContext->GSSetShader(nullptr, nullptr, 0);
		}
	};

// Global Variables
	CModelViewerCamera m_Camera;
	RenderableMesh m_Mesh;
	D3DXMATRIX m_WorldMatrix;
	std::auto_ptr<VSPreObjectConstBuf> m_VsPreObjectConstBuf;
	std::auto_ptr<PSPreObjectConstBuf> m_PsPreObjectConstBuf;
	std::auto_ptr<PSPostProcessConstBuf> m_PSPostProcessConstBuf;
	//ID3D11SamplerState* m_SamplerState;
	js::SamplerState m_SamplerState;
	js::DepthStencilState m_DepthStencilState;
	Rendering m_Rendering;
	Histogram m_Histogram;
	bool m_ShowHistogram;
	
	// post processing
	ScreenQuad m_ScreenQuad;
	js::VertexShader m_PostVtxShd;
	js::PixelShader m_PostFogShd;

// Methods
	Example02()
		: m_ShowHistogram(true)
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

		m_SamplerState.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerState.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerState.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerState.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		m_SamplerState.create(d3dDevice);

		m_DepthStencilState.DepthEnable = FALSE;
		m_DepthStencilState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		m_DepthStencilState.DepthFunc = D3D11_COMPARISON_ALWAYS;
		m_DepthStencilState.StencilEnable = FALSE;
		m_DepthStencilState.create(d3dDevice);

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

		m_Histogram.create(d3dDevice);

		// post processing
		m_PostVtxShd.createFromFile(d3dDevice, media(L"Example02/Post.Vtx.hlsl"), "Main");
		js_assert(m_PostVtxShd.valid());

		m_PostFogShd.createFromFile(d3dDevice, media(L"Example02/Post.DOF.hlsl"), "Main");
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

		m_Camera.SetProjParams( D3DX_PI / 4, aspect, 1.0f, radius * 8.0f );
		m_Camera.SetWindow( backBufferSurfaceDesc->Width, backBufferSurfaceDesc->Height );
		m_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );

		m_Rendering.onSwapChainResized(d3dDevice, backBufferSurfaceDesc->Width, backBufferSurfaceDesc->Height);

		return S_OK;
	}

	__override void onD3D11ReleasingSwapChain()
	{
		guiOnD3D11ReleasingSwapChain();

		m_Rendering.onSwapChainReleasing();
	}

	__override void onD3D11DestroyDevice()
	{
		guiOnD3D11DestroyDevice();

		DXUTGetGlobalResourceCache().OnDestroyDevice();
		m_Mesh.destroy();
		m_VsPreObjectConstBuf.reset();
		m_PsPreObjectConstBuf.reset();

		m_Histogram.destroy();

		m_PSPostProcessConstBuf.reset();
		
		m_ScreenQuad.destroy();
		m_PostVtxShd.destroy();
		m_PostFogShd.destroy();
		
		m_SamplerState.destroy();
		m_DepthStencilState.destroy();
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
			d3dImmediateContext->PSSetSamplers(0, 1, &m_SamplerState.m_StateObject);

			m_Mesh.render(d3dImmediateContext);

		}	m_Rendering.unprepareRenderScene(d3dDevice, d3dImmediateContext);

		// Post-Processing
		d3dImmediateContext->OMSetDepthStencilState(m_DepthStencilState, 0);

		// update histogram
		if(m_ShowHistogram)
			m_Histogram.update(d3dImmediateContext, m_Rendering.m_ColorBufferQuater);
		
		{	
			// m_PSPostProcessConstBuf
			m_PSPostProcessConstBuf->map(d3dImmediateContext);
			m_PSPostProcessConstBuf->data().update(m_Camera);
			m_PSPostProcessConstBuf->unmap(d3dImmediateContext);

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

			{	// reset PS shader resources
				ID3D11ShaderResourceView* shv[] = {nullptr, nullptr};
				d3dImmediateContext->PSSetShaderResources(0, 2, shv);
			}
		}

		if(m_ShowHistogram)
			m_Histogram.display(d3dImmediateContext, -0.9f, -0.5f, 0.5f, 0.25f);

		d3dImmediateContext->OMSetDepthStencilState(nullptr, 0);

		// gui
		m_GuiTxtHelper->Begin();
		m_GuiTxtHelper->SetInsertionPos( 2, 0 );
		m_GuiTxtHelper->SetForegroundColor( D3DXCOLOR( 0.2f, 0.2f, 1.0f, 1.0f ) );
		m_GuiTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
		m_GuiTxtHelper->End();
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


