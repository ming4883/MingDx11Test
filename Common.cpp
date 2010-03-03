#include "DXUT.h"
#include "Common.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")


#include "SDKmisc.h"	// DXUTFindDXSDKMediaFileCch
#include "SDKmesh.h"	// DXUT
#include "Jessye/Shaders.h"
#include "Jessye/Buffers.h"

const wchar_t* media(const wchar_t* in)
{
	static WCHAR out[MAX_PATH];
	HRESULT hr;
	if(FAILED(hr = DXUTFindDXSDKMediaFileCch(out, sizeof(out) / sizeof(wchar_t), in))) {
		wsprintf(out, L"media %s not found", in);
		DXUTTrace(__FILE__, __LINE__, hr, out, false);
		return in;
	}

	// convert all '/' to '\\'
	wchar_t* ch = out;
	while(*ch != 0)
	{
		if(*ch == '/') *ch = '\\';
		++ch;
	}

	return out;
}

void inputElement(
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

//------------------------------------------------------------------------------
// RenderableMesh
//------------------------------------------------------------------------------
class RenderableMesh::Impl
{
public:
	mutable CDXUTSDKMesh m_Mesh;
	std::auto_ptr<js::VertexShader> m_VS;
	std::auto_ptr<js::PixelShader> m_PS;
	ID3D11InputLayout* m_IL;

	Impl()
		: m_IL(nullptr)
	{
	}

	~Impl()
	{
		destroy();
	}

	bool create(
		ID3D11Device* d3dDevice,
		const wchar_t* meshPath,
		const RenderableMesh::ShaderDesc& shaderDesc,
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

		m_IL = js::Buffers::createInputLayout(d3dDevice, &inputElems[0], inputElems.size(), m_VS->m_ByteCode);
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

	void render(ID3D11DeviceContext* d3dContext) const
	{
		// input layout
		d3dContext->IASetInputLayout(m_IL);

		// shaders
		d3dContext->VSSetShader(m_VS->m_ShaderObject, nullptr, 0);
		d3dContext->PSSetShader(m_PS->m_ShaderObject, nullptr, 0);

		// render mesh
		m_Mesh.Render(d3dContext, 0);
	}

	float radius() const
	{
#define _max(a, b) (a > b ? a : b)
		float r = 0;
		for(UINT i=0; i<m_Mesh.GetNumMeshes(); ++i)
		{
			D3DXVECTOR3 e = m_Mesh.GetMeshBBoxExtents(i);
			r = _max(r, _max(e.x, _max(e.y, e.z)));
		}

		return r;
#undef _max
	}

};	// RenderableMesh::Impl

RenderableMesh::RenderableMesh() : m_Impl(*new Impl)
{
}

RenderableMesh::~RenderableMesh()
{
	delete &m_Impl;
}

bool RenderableMesh::create(
	ID3D11Device* d3dDevice,
	const wchar_t* meshPath,
	const ShaderDesc& shaderDesc,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElems
	)
{
	return m_Impl.create(d3dDevice, meshPath, shaderDesc, inputElems);
}

void RenderableMesh::destroy()
{
	m_Impl.destroy();
}

void RenderableMesh::render(ID3D11DeviceContext* d3dContext) const
{
	m_Impl.render(d3dContext);
}

float RenderableMesh::radius() const
{
	return m_Impl.radius();
}

//------------------------------------------------------------------------------
// ScreenQuad
//------------------------------------------------------------------------------
class ScreenQuad::Impl
{
public:
	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;
	ID3D11InputLayout* m_IL;

	Impl() : m_VB(nullptr), m_IB(nullptr), m_IL(nullptr)
	{
	}

	static D3DXVECTOR3 v[];
	static unsigned short i[];

};	// ScreenQuad::Impl

D3DXVECTOR3 ScreenQuad::Impl::v[] = {
	D3DXVECTOR3( 1, 1, 0),
	D3DXVECTOR3( 1,-1, 0),
	D3DXVECTOR3(-1, 1, 0),
	D3DXVECTOR3(-1,-1, 0),
};

unsigned short ScreenQuad::Impl::i[] = {
	0, 1, 2, 3, 2, 1
};

ScreenQuad::ScreenQuad()
	: m_Impl(*new Impl)
{
}

ScreenQuad::~ScreenQuad()
{
	delete &m_Impl;
}

bool ScreenQuad::valid() const
{
	return m_Impl.m_VB != nullptr;
}

void ScreenQuad::create(ID3D11Device* d3dDevice, ID3DBlob* shaderByteCode)
{
	m_Impl.m_VB = js::Buffers::createVertexBuffer(d3dDevice, sizeof(Impl::v), sizeof(Impl::v[0]), false, Impl::v);
	js_assert(m_Impl.m_VB != nullptr);

	m_Impl.m_IB = js::Buffers::createIndexBuffer(d3dDevice, sizeof(Impl::i), sizeof(Impl::i[0]), false, Impl::i);
	js_assert(m_Impl.m_IB != nullptr);

	std::vector<D3D11_INPUT_ELEMENT_DESC> ielems;
	inputElement(ielems, "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0);

	m_Impl.m_IL = js::Buffers::createInputLayout(d3dDevice, &ielems[0], ielems.size(), shaderByteCode);
	js_assert(m_Impl.m_IL != nullptr);

	if(nullptr == m_Impl.m_VB || nullptr == m_Impl.m_IB || nullptr == m_Impl.m_IL)
	{
		destroy();
		return;
	}

}

void ScreenQuad::destroy()
{
	js_safe_release(m_Impl.m_VB);
	js_safe_release(m_Impl.m_IB);
	js_safe_release(m_Impl.m_IL);
}

void ScreenQuad::render(ID3D11DeviceContext* d3dContext) const
{
	UINT strides[] = {sizeof(Impl::v[0])};
	UINT offsets[] = {0};

	d3dContext->IASetInputLayout(m_Impl.m_IL);
	d3dContext->IASetIndexBuffer(m_Impl.m_IB, ::DXGI_FORMAT_R16_UINT, 0);
	d3dContext->IASetVertexBuffers(0, 1, &m_Impl.m_VB, strides, offsets);
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dContext->DrawIndexed(6, 0, 0);
}

//------------------------------------------------------------------------------
// DXUTApp
//------------------------------------------------------------------------------
#define DECL_DXUT_APP(userContext) DXUTApp& app = *static_cast<DXUTApp*>(userContext);

// Reject any D3D11 devices that aren't acceptable by returning false
bool CALLBACK IsD3D11DeviceAcceptable(
	const CD3D11EnumAdapterInfo *AdapterInfo,
	UINT Output,
	const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat,
	bool bWindowed,
	void* pUserContext)
{
	DECL_DXUT_APP(pUserContext);
	return app.isD3D11DeviceAcceptable(AdapterInfo, Output, DeviceInfo, BackBufferFormat, bWindowed);
}

// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
bool CALLBACK ModifyDeviceSettings(
	DXUTDeviceSettings* pDeviceSettings,
	void* pUserContext)
{
	DECL_DXUT_APP(pUserContext);
	return app.modifyDeviceSettings(pDeviceSettings);
}

// Call if device was removed.  Return true to find a new device, false to quit
bool CALLBACK OnDeviceRemoved(void* pUserContext)
{
	DECL_DXUT_APP(pUserContext);
    return app.onDeviceRemoved();
}

// Create any D3D11 resources that aren't dependant on the back buffer
HRESULT CALLBACK OnD3D11CreateDevice(
	ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	DECL_DXUT_APP(pUserContext);
	return app.onD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc);
}

// Create any D3D11 resources that depend on the back buffer
HRESULT CALLBACK OnD3D11ResizedSwapChain(
	ID3D11Device* pd3dDevice,
	IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	DECL_DXUT_APP(pUserContext);
	return app.onD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc);
}

// Release D3D11 resources created in OnD3D11ResizedSwapChain 
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	DECL_DXUT_APP(pUserContext);
	app.onD3D11ReleasingSwapChain();
}

// Release D3D11 resources created in OnD3D11CreateDevice 
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	DECL_DXUT_APP(pUserContext);
	app.onD3D11DestroyDevice();
}

// Handle updates to the scene.  This is called regardless of which D3D API is used
void CALLBACK OnFrameMove(
	double fTime,
	float fElapsedTime,
	void* pUserContext)
{
	DECL_DXUT_APP(pUserContext);
	app.onFrameMove(fTime, fElapsedTime);
}

// Render the scene using the D3D11 device
void CALLBACK OnD3D11FrameRender(
	ID3D11Device* pd3dDevice,
	ID3D11DeviceContext* pd3dImmediateContext,
	double fTime,
	float fElapsedTime,
	void* pUserContext)
{
	DECL_DXUT_APP(pUserContext);
	app.onD3D11FrameRender(pd3dDevice, pd3dImmediateContext, fTime, fElapsedTime);
}

// Handle messages to the application
LRESULT CALLBACK MsgProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	bool* pbNoFurtherProcessing,
	void* pUserContext)
{
	DECL_DXUT_APP(pUserContext);
    return app.msgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing);
}

// Handle key presses
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	DECL_DXUT_APP(pUserContext);
	app.onKeyboard(nChar, bKeyDown, bAltDown);
}

// Handle mouse button presses
void CALLBACK OnMouse(
	bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
	bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
	int xPos, int yPos, void* pUserContext)
{
	DECL_DXUT_APP(pUserContext);
	app.onMouse(
		bLeftButtonDown, bRightButtonDown, bMiddleButtonDown,
		bSideButton1Down, bSideButton2Down, nMouseWheelDelta,
		xPos, yPos);
}

// Initialize everything and go into a render loop
int DXUTApp::run(DXUTApp& app)
{
    // Set general DXUT callbacks
    DXUTSetCallbackFrameMove( OnFrameMove, &app );
    DXUTSetCallbackKeyboard( OnKeyboard, &app );
    DXUTSetCallbackMouse( OnMouse, false, &app );
    DXUTSetCallbackMsgProc( MsgProc, &app );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings, &app );
    DXUTSetCallbackDeviceRemoved( OnDeviceRemoved, &app );

    // Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable, &app );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice, &app );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain, &app );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender, &app );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain, &app );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice, &app );

    // Perform any application-level initialization here
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( app.getName() );

    // Only require 10-level hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 640, 480 );
    DXUTMainLoop(); // Enter into the DXUT ren  der loop

    // Perform any application-level cleanup here
	int exitCode = DXUTGetExitCode();
	DXUTDestroyState();

    return exitCode;
}