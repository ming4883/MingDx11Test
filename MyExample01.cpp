//--------------------------------------------------------------------------------------
// File: MyExample01.cpp
//
// Empty starting point for new Direct3D 9 and/or Direct3D 11 applications
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmesh.h"
#include "SDKmisc.h"	// DXUTFindDXSDKMediaFileCch

#include "Jessye/Shaders.h"
#include "Jessye/Buffers.h"

#include <vector>
#include <string>

#pragma comment(lib, "d3dcompiler.lib")

static const wchar_t* media(const wchar_t* in)
{
	static WCHAR out[MAX_PATH];
	HRESULT hr;
	if(FAILED(hr = DXUTFindDXSDKMediaFileCch(out, sizeof(out) / sizeof(wchar_t), in))) {
		wsprintf(out, L"media %s not found", in);
		DXUTTrace(__FILE__, __LINE__, hr, out, false);
		return in;
	}

	return out;
}

static void input_element(
	std::vector<D3D11_INPUT_ELEMENT_DESC>& elems,
	LPCSTR semanticName,
    UINT semanticIndex,
    DXGI_FORMAT format,
    UINT inputSlot,
    UINT alignedByteOffset,
    D3D11_INPUT_CLASSIFICATION inputSlotClass,
    UINT instanceDataStepRate)
{

	D3D11_INPUT_ELEMENT_DESC desc;
	desc.SemanticName = semanticName;
	desc.SemanticIndex = semanticIndex;
	desc.Format = format;
	desc.InputSlot = inputSlot;
	desc.AlignedByteOffset = alignedByteOffset;
	desc.InputSlotClass = inputSlotClass;
	desc.InstanceDataStepRate = instanceDataStepRate;

	elems.push_back(desc);
}

class RenderableMesh
{
public:
	CDXUTSDKMesh m_Mesh;
	std::auto_ptr<js::VertexShader> m_VS;
	std::auto_ptr<js::PixelShader> m_PS;
	ID3D11InputLayout* m_IL;

	RenderableMesh()
		: m_IL(nullptr)
	{
	}

	~RenderableMesh()
	{
		destroy();
	}

	struct ShaderDesc
	{
		std::wstring vsPath;
		std::wstring psPath;
		std::string vsEntry;
		std::string psEntry;
	};	// ShaderDesc

	bool create(
		ID3D11Device* d3dDevice,
		const wchar_t* meshPath,
		const ShaderDesc& shaderDesc,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElems
		)
	{
		HRESULT hr;

		hr = m_Mesh.Create(d3dDevice, meshPath, false);
		js_assert(SUCCEEDED(hr));

		m_VS.reset(new js::VertexShader);
		m_VS->createFromFile(d3dDevice, shaderDesc.vsPath.c_str(), shaderDesc.vsEntry.c_str());
		js_assert(m_VS->valid());

		m_PS.reset(new js::PixelShader);
		m_PS->createFromFile(d3dDevice, shaderDesc.psPath.c_str(), shaderDesc.psEntry.c_str());
		js_assert(m_PS->valid());

		m_IL = js::Buffers::createInputLayout(d3dDevice, &inputElems[0], inputElems.size(), *m_VS);
		js_assert(m_IL != nullptr);

		return true;
	}

	void destroy()
	{
		m_Mesh.Destroy();
		
		m_VS.reset();
		m_PS.reset();

		js_safe_release(m_IL);
	}

	void render(ID3D11DeviceContext* d3dContext)
	{
		const UINT meshIt = 0;

		// input layout
		d3dContext->IASetInputLayout(m_IL);

		// buffers
		UINT strides[1];
		UINT offsets[1];
		ID3D11Buffer* vbs[1];

		vbs[0] = m_Mesh.GetVB11(meshIt, 0);
		strides[0] = (UINT)m_Mesh.GetVertexStride(meshIt, 0);
		offsets[0] = 0;

		d3dContext->IASetVertexBuffers(0, 1, vbs, strides, offsets);
		d3dContext->IASetIndexBuffer(m_Mesh.GetIB11(meshIt), m_Mesh.GetIBFormat11(meshIt), 0);

		// shaders
		d3dContext->VSSetShader(m_VS->m_ShaderObject, nullptr, 0);
		d3dContext->PSSetShader(m_PS->m_ShaderObject, nullptr, 0);

		for(UINT subsetIt = 0; subsetIt < m_Mesh.GetNumSubsets(meshIt); ++subsetIt)
		{
			// Get the subset
			SDKMESH_SUBSET* subset = m_Mesh.GetSubset(meshIt, subsetIt);
			D3D11_PRIMITIVE_TOPOLOGY primType = CDXUTSDKMesh::GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)subset->PrimitiveType);

			d3dContext->IASetPrimitiveTopology(primType);
			d3dContext->DrawIndexed((UINT)subset->IndexCount, 0, (UINT)subset->VertexStart);
		}
	}

};	// RenderableMesh

#pragma pack(push)
#pragma pack(1)

struct CB_VS_PER_OBJECT
{
    D3DXMATRIX m_WorldViewProj;
    D3DXMATRIX m_World;
};

struct CB_PS_PER_OBJECT
{
    D3DXVECTOR4 m_vObjectColor;
};
#pragma pack(pop)


//--------------------------------------------------------------------------------------
CModelViewerCamera g_Camera;
RenderableMesh g_Mesh;
D3DXMATRIX g_World;
D3DXMATRIX g_WorldViewProjection;
std::auto_ptr< js::ConstantBuffer_t<CB_VS_PER_OBJECT> > g_CbVsPreObject;
std::auto_ptr< js::ConstantBuffer_t<CB_PS_PER_OBJECT> > g_CbPsPreObject;


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(
	const CD3D11EnumAdapterInfo *AdapterInfo,
	UINT Output,
	const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat,
	bool bWindowed,
	void* pUserContext)
{
    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(
	DXUTDeviceSettings* pDeviceSettings,
	void* pUserContext)
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(
	ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	//HRESULT hr;

	// init camera
	const float radius = 600.0f;
	D3DXVECTOR3 vecEye( 0.0f, 0.0f, -100.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
    g_Camera.SetViewParams( &vecEye, &vecAt );
    g_Camera.SetRadius(radius, radius, radius);

	// load mesh
	RenderableMesh::ShaderDesc sd;
	sd.vsPath = media(L"MyExample01/BasicHLSL11_VS.hlsl");
	sd.vsEntry = "VSMain";

	sd.psPath = media(L"MyExample01/BasicHLSL11_PS.hlsl");
	sd.psEntry = "PSMain";

	std::vector<D3D11_INPUT_ELEMENT_DESC> ielems;
	input_element(ielems, "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0);
    input_element(ielems, "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0);
    input_element(ielems, "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0);

	g_Mesh.create(pd3dDevice, media(L"Tiny/tiny.sdkmesh"), sd, ielems);

	g_CbVsPreObject.reset(new js::ConstantBuffer_t<CB_VS_PER_OBJECT>);
	g_CbVsPreObject->create(pd3dDevice);

	g_CbPsPreObject.reset(new js::ConstantBuffer_t<CB_PS_PER_OBJECT>);
	g_CbPsPreObject->create(pd3dDevice);
	
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(
	ID3D11Device* pd3dDevice,
	IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 2.0f, 4000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(
	double fTime,
	float fElapsedTime,
	void* pUserContext)
{
	g_Camera.FrameMove( fElapsedTime );

	D3DXMatrixIdentity(&g_World);
    g_WorldViewProjection = g_World * *g_Camera.GetViewMatrix() * *g_Camera.GetProjMatrix();
	
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(
	ID3D11Device* pd3dDevice,
	ID3D11DeviceContext* pd3dImmediateContext,
	double fTime,
	float fElapsedTime,
	void* pUserContext)
{
    // Clear render target and the depth stencil 
    //float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };
	float ClearColor[4] = { 0.176f, 0.176f, 0.176f, 0.0f };

    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	// g_CbVsPreObject
	g_CbVsPreObject->map(pd3dImmediateContext);
	D3DXMatrixTranspose( &g_CbVsPreObject->data().m_WorldViewProj, &g_WorldViewProjection );
    D3DXMatrixTranspose( &g_CbVsPreObject->data().m_World, &g_World );
	g_CbVsPreObject->unmap(pd3dImmediateContext);

	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_CbVsPreObject->m_BufferObject);

	// g_CbPsPreObject
	g_CbPsPreObject->map(pd3dImmediateContext);
	g_CbPsPreObject->data().m_vObjectColor = D3DXVECTOR4(1, 1, 0, 1);
	g_CbPsPreObject->unmap(pd3dImmediateContext);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &g_CbPsPreObject->m_BufferObject);

	g_Mesh.render(pd3dImmediateContext);
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	g_Mesh.destroy();
	g_CbVsPreObject.reset();
	g_CbPsPreObject.reset();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	bool* pbNoFurtherProcessing,
	void* pUserContext)
{
	g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                       int xPos, int yPos, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved(void* pUserContext)
{
    return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set general DXUT callbacks
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackMouse( OnMouse );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );

    // Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    // Perform any application-level initialization here

    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"MyExample01" );

    // Only require 10-level hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 640, 480 );
    DXUTMainLoop(); // Enter into the DXUT ren  der loop

    // Perform any application-level cleanup here

    return DXUTGetExitCode();
}


