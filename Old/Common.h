#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include "Jessye/Platform.h"
#include "Jessye/RenderStates.h"
#include "Jessye/Buffers.h"
#include "Jessye/Shaders.h"
#include "Jessye/ViewArrays.h"

const wchar_t* media(const wchar_t* in);

void inputElement(
	std::vector<D3D11_INPUT_ELEMENT_DESC>& elems,
	LPCSTR semanticName,
    UINT semanticIndex,
    DXGI_FORMAT format,
    UINT inputSlot,
    UINT alignedByteOffset,
    D3D11_INPUT_CLASSIFICATION inputSlotClass,
    UINT instanceDataStepRate);

class RenderableMesh
{
public:
	struct ShaderDesc
	{
		std::wstring vsPath;
		std::wstring psPath;
		std::string vsEntry;
		std::string psEntry;
	};	// ShaderDesc

	RenderableMesh();

	~RenderableMesh();
	
	bool create(
		ID3D11Device* d3dDevice,
		const wchar_t* meshPath,
		const ShaderDesc& shaderDesc,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElems
		);

	void destroy();

	void render(ID3D11DeviceContext* d3dContext, js::RenderStateCache* rsCache, size_t numInstances = 1) const;

	float radius() const;

private:
	class Impl; Impl& m_Impl;

	js_decl_non_copyable(RenderableMesh);

};	// RenderableMesh

class ScreenQuad
{
public:
	ScreenQuad();

	~ScreenQuad();

	bool valid() const;

	void create(ID3D11Device* d3dDevice, ID3DBlob* shaderByteCode);

	void destroy();

	void render(ID3D11DeviceContext* d3dContext) const;

private:
	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;
	ID3D11InputLayout* m_IL;

	js_decl_non_copyable(ScreenQuad);
};	// ScreenQuad

class PointMesh
{
public:
	PointMesh();

	~PointMesh();

	bool valid() const;

	void create(ID3D11Device* d3dDevice, ID3DBlob* shaderByteCode);

	void destroy();

	void render(ID3D11DeviceContext* d3dContext, size_t instanceCount) const;

private:
	ID3D11Buffer* m_VB;
	ID3D11InputLayout* m_IL;

	js_decl_non_copyable(PointMesh);
};	// PointMesh

class CBaseCamera;

class PostProcessor
{
public:
	struct Constants
	{
		D3DXMATRIX m_InvViewProjScaleBias;
		D3DXVECTOR4 m_ZParams;
		D3DXVECTOR4 m_UserParams;

		void update(const CBaseCamera& camera);
	};

	typedef js::ConstantBuffer_t<Constants> ConstBuffer;

	ScreenQuad m_ScreenQuad;
	js::VertexShader m_PostVtxShd;
	ConstBuffer m_ConstBuf;

	bool valid() const;

	void create(ID3D11Device* d3dDevice);

	void destroy();

	void filter(
		ID3D11DeviceContext* d3dContext,
		js::RenderStateCache& rsCache,
		js::RenderBuffer& srcBuf,
		js::RenderBuffer& dstBuf,
		js::PixelShader& shader
		);

	void filter(
		ID3D11DeviceContext* d3dContext,
		js::RenderStateCache& rsCache,
		js::SrvVA& srvVA,
		js::RenderBuffer& dstBuf,
		js::PixelShader& shader
		);

	void filter(
		ID3D11DeviceContext* d3dContext,
		js::RenderStateCache& rsCache,
		js::SrvVA& srvVA,
		js::RtvVA& rtvVA,
		const D3D11_VIEWPORT& vp,
		js::PixelShader& shader
		);

	void filter(
		ID3D11DeviceContext* d3dContext,
		js::RenderStateCache& rsCache,
		js::SrvVA& srvVA,
		js::PixelShader& shader
		);
};

struct SceneShaderConstants
{
#pragma pack(push)
#pragma pack(1)
	D3DXMATRIX m_World;
	D3DXMATRIX m_View;
	D3DXMATRIX m_Projection;
	D3DXMATRIX m_ViewProjection;
	D3DXVECTOR4 m_CameraPosition;
	D3DXVECTOR4 m_CameraParams;
	D3DXVECTOR4 m_AmbientColor;
	D3DXVECTOR4 m_LightVector;
	D3DXVECTOR4 m_LightColor;
#pragma pack(pop)

	void updateCameraContants(const CBaseCamera& camera);
};

typedef js::ConstantBuffer_t<SceneShaderConstants> SceneShaderConstantsBuffer;

class CDXUTDialogResourceManager;
class CDXUTTextHelper;
class CDXUTDialog;

class DXUTApp
{
public:
	virtual ~DXUTApp();

	static int run(DXUTApp& app);

	virtual const wchar_t* getName() = 0;

	// Reject any D3D11 devices that aren't acceptable by returning false
	virtual bool isD3D11DeviceAcceptable(
		const CD3D11EnumAdapterInfo* adapterInfo,
		UINT output,
		const CD3D11EnumDeviceInfo* deviceInfo,
		DXGI_FORMAT backBufferFormat,
		bool windowed)
	{
		return true;
	}

	// Called right before creating a D3D11 device, 
	// allowing the app to modify the device settings as needed
	virtual bool modifyDeviceSettings(DXUTDeviceSettings* deviceSettings)
	{
		return true;
	}

	// Call if device was removed.  Return true to find a new device, false to quit
	virtual bool onDeviceRemoved()
	{
		return true;
	}

	// Create any D3D11 resources that aren't dependant on the back buffer
	virtual HRESULT onD3D11CreateDevice(
		ID3D11Device* d3dDevice,
		const DXGI_SURFACE_DESC* backBufferSurfaceDesc)
	{
		return S_OK;
	}

	// Create any D3D11 resources that depend on the back buffer
	virtual HRESULT onD3D11ResizedSwapChain(
		ID3D11Device* d3dDevice,
		IDXGISwapChain* swapChain,
		const DXGI_SURFACE_DESC* backBufferSurfaceDesc)
	{
		return S_OK;
	}

	// Release D3D11 resources created in OnD3D11ResizedSwapChain
	virtual void onD3D11ReleasingSwapChain()
	{
	}

	// Handle updates to the scene.  This is called regardless of which D3D API is used
	virtual void onFrameMove(
		double time,
		float elapsedTime)
	{
	}

	// Release D3D11 resources created in OnD3D11CreateDevice 
	virtual void onD3D11DestroyDevice()
	{
	}

	// Render the scene using the D3D11 device
	virtual void onD3D11FrameRender(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dImmediateContext,
		double time,
		float elapsedTime)
	{
	}

	// Handle messages to the application
	virtual LRESULT msgProc(
		HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing)
	{
		return 0;
	}

	// Handle key presses
	virtual void onKeyboard(UINT nChar, bool keyDown, bool altDown)
	{
	}

	// Handle mouse button presses
	virtual void onMouse(
		bool leftButtonDown, bool rightButtonDown, bool middleButtonDown,
		bool sideButton1Down, bool sideButton2Down, int mouseWheelDelta,
		int xPos, int yPos)
	{
	}

protected:
	CDXUTDialogResourceManager* m_GuiDlgResMgr;
	CDXUTTextHelper* m_GuiTxtHelper;
	std::vector<CDXUTDialog*> m_GuiDlgs;

	DXUTApp();

	void guiOnD3D11CreateDevice(ID3D11Device* d3dDevice);
	void guiOnD3D11DestroyDevice();
	void guiOnD3D11ResizedSwapChain(ID3D11Device* d3dDevice, const DXGI_SURFACE_DESC* backBufferSurfaceDesc);
	void guiOnD3D11ReleasingSwapChain();
	void guiUpdateStaticWithSlider(size_t dlgId, int staticId, int sliderId, const wchar_t* text);
	int guiMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing);
};

#endif	// COMMON_H