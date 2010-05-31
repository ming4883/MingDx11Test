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
#include "DXUTgui.h"
#include "DXUTcamera.h"
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
class MySDKMesh : public CDXUTSDKMesh
{
public:
	enum {MAX_D3D11_VERTEX_STREAMS = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT};

	__override ~MySDKMesh() {}

	void render(
		ID3D11DeviceContext* pd3dDeviceContext,
		js::RenderStateCache* rsCache,
		UINT iNumInstances = 1,
		UINT iDiffuseSlot = (UINT)-1,
		UINT iNormalSlot = (UINT)-1,
		UINT iSpecularSlot = (UINT)-1)
	{
		renderFrame(0, false, pd3dDeviceContext, rsCache, iNumInstances, iDiffuseSlot, iNormalSlot, iSpecularSlot);
	}

	void renderFrame( 
		UINT iFrame,
		bool bAdjacent,
		ID3D11DeviceContext* pd3dDeviceContext,
		js::RenderStateCache* rsCache,
		UINT iNumInstances,
		UINT iDiffuseSlot,
		UINT iNormalSlot,
		UINT iSpecularSlot)
	{
		if( !m_pStaticMeshData || !m_pFrameArray )
			return;

		if( m_pFrameArray[iFrame].Mesh != INVALID_MESH )
		{
			renderMesh(
				m_pFrameArray[iFrame].Mesh,
				bAdjacent,
				pd3dDeviceContext,
				rsCache,
				iNumInstances,
				iDiffuseSlot,
				iNormalSlot,
				iSpecularSlot);
		}

		// Render our children
		if( m_pFrameArray[iFrame].ChildFrame != INVALID_FRAME )
			renderFrame( 
				m_pFrameArray[iFrame].ChildFrame, bAdjacent, pd3dDeviceContext, rsCache, iNumInstances, iDiffuseSlot, 
				iNormalSlot, iSpecularSlot );

		// Render our siblings
		if( m_pFrameArray[iFrame].SiblingFrame != INVALID_FRAME )
			renderFrame(
				m_pFrameArray[iFrame].SiblingFrame, bAdjacent, pd3dDeviceContext, rsCache, iNumInstances, iDiffuseSlot, 
				iNormalSlot, iSpecularSlot );
	}

	void renderMesh( 
		UINT iMesh,
		bool bAdjacent,
		ID3D11DeviceContext* pd3dDeviceContext,
		js::RenderStateCache* rsCache,
		UINT iNumInstances,
		UINT iDiffuseSlot,
		UINT iNormalSlot,
		UINT iSpecularSlot)
	{
		if( 0 < GetOutstandingBufferResources() )
			return;

		SDKMESH_MESH* pMesh = &m_pMeshArray[iMesh];

		UINT Strides[MAX_D3D11_VERTEX_STREAMS];
		UINT Offsets[MAX_D3D11_VERTEX_STREAMS];
		ID3D11Buffer* pVB[MAX_D3D11_VERTEX_STREAMS];

		if( pMesh->NumVertexBuffers > MAX_D3D11_VERTEX_STREAMS )
			return;

		for( UINT64 i = 0; i < pMesh->NumVertexBuffers; i++ )
		{
			pVB[i] = m_pVertexBufferArray[ pMesh->VertexBuffers[i] ].pVB11;
			Strides[i] = ( UINT )m_pVertexBufferArray[ pMesh->VertexBuffers[i] ].StrideBytes;
			Offsets[i] = 0;
		}

		SDKMESH_INDEX_BUFFER_HEADER* pIndexBufferArray;
		if( bAdjacent )
			pIndexBufferArray = m_pAdjacencyIndexBufferArray;
		else
			pIndexBufferArray = m_pIndexBufferArray;

		ID3D11Buffer* pIB = pIndexBufferArray[ pMesh->IndexBuffer ].pIB11;
		DXGI_FORMAT ibFormat = DXGI_FORMAT_R16_UINT;
		switch( pIndexBufferArray[ pMesh->IndexBuffer ].IndexType )
		{
		case IT_16BIT:
			ibFormat = DXGI_FORMAT_R16_UINT;
			break;
		case IT_32BIT:
			ibFormat = DXGI_FORMAT_R32_UINT;
			break;
		};

		pd3dDeviceContext->IASetVertexBuffers( 0, pMesh->NumVertexBuffers, pVB, Strides, Offsets );
		pd3dDeviceContext->IASetIndexBuffer( pIB, ibFormat, 0 );

		SDKMESH_SUBSET* pSubset = NULL;
		SDKMESH_MATERIAL* pMat = NULL;
		D3D11_PRIMITIVE_TOPOLOGY PrimType;

		for( UINT subset = 0; subset < pMesh->NumSubsets; subset++ )
		{
			pSubset = &m_pSubsetArray[ pMesh->pSubsets[subset] ];

			PrimType = GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
			if( bAdjacent )
			{
				switch( PrimType )
				{
				case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
					PrimType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
					break;
				case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
					PrimType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
					break;
				case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
					PrimType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
					break;
				case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
					PrimType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
					break;
				}
			}

			pd3dDeviceContext->IASetPrimitiveTopology( PrimType );

			pMat = &m_pMaterialArray[ pSubset->MaterialID ];
			if(nullptr != rsCache)
			{
				if( iDiffuseSlot != INVALID_SAMPLER_SLOT && !IsErrorResource( pMat->pDiffuseRV11 ) )
					rsCache->psState().setSRViews( iDiffuseSlot, 1, &pMat->pDiffuseRV11 );
				if( iNormalSlot != INVALID_SAMPLER_SLOT && !IsErrorResource( pMat->pNormalRV11 ) )
					rsCache->psState().setSRViews( iNormalSlot, 1, &pMat->pNormalRV11 );
				if( iSpecularSlot != INVALID_SAMPLER_SLOT && !IsErrorResource( pMat->pSpecularRV11 ) )
					rsCache->psState().setSRViews( iSpecularSlot, 1, &pMat->pSpecularRV11 );
			}
			else
			{
				if( iDiffuseSlot != INVALID_SAMPLER_SLOT && !IsErrorResource( pMat->pDiffuseRV11 ) )
					pd3dDeviceContext->PSSetShaderResources( iDiffuseSlot, 1, &pMat->pDiffuseRV11 );
				if( iNormalSlot != INVALID_SAMPLER_SLOT && !IsErrorResource( pMat->pNormalRV11 ) )
					pd3dDeviceContext->PSSetShaderResources( iNormalSlot, 1, &pMat->pNormalRV11 );
				if( iSpecularSlot != INVALID_SAMPLER_SLOT && !IsErrorResource( pMat->pSpecularRV11 ) )
					pd3dDeviceContext->PSSetShaderResources( iSpecularSlot, 1, &pMat->pSpecularRV11 );
			}

			UINT IndexCount = ( UINT )pSubset->IndexCount;
			UINT IndexStart = ( UINT )pSubset->IndexStart;
			UINT VertexStart = ( UINT )pSubset->VertexStart;
			if( bAdjacent )
			{
				IndexCount *= 2;
				IndexStart *= 2;
			}

			if(nullptr != rsCache)
				rsCache->applyToContext(pd3dDeviceContext);

			if(iNumInstances == 1)
				pd3dDeviceContext->DrawIndexed( IndexCount, IndexStart, VertexStart );
			else
				pd3dDeviceContext->DrawIndexedInstanced( IndexCount, iNumInstances, IndexStart, VertexStart, 0 );
		}
	}

};	// MySDKMesh

class RenderableMesh::Impl
{
public:
	mutable MySDKMesh m_Mesh;
	js::VertexShader m_VS;
	js::PixelShader m_PS;
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

		m_VS.createFromFile(d3dDevice, shaderDesc.vsPath.c_str(), shaderDesc.vsEntry.c_str());
		js_assert(m_VS.valid());

		m_PS.createFromFile(d3dDevice, shaderDesc.psPath.c_str(), shaderDesc.psEntry.c_str());
		js_assert(m_PS.valid());

		m_IL = js::Buffers::createInputLayout(d3dDevice, &inputElems[0], inputElems.size(), m_VS.m_ByteCode);
		js_assert(m_IL != nullptr);

		return true;
	}

	void destroy()
	{
		m_Mesh.Destroy();
		
		m_VS.destroy();
		m_PS.destroy();

		js_safe_release(m_IL);
	}

	void render(ID3D11DeviceContext* d3dContext, js::RenderStateCache* rsCache, size_t numInstances) const
	{
		// input layout
		d3dContext->IASetInputLayout(m_IL);

		// shaders
		if(nullptr != rsCache)
		{
			rsCache->vsState().setShader(m_VS);
			rsCache->psState().setShader(m_PS);
		}
		else
		{
			d3dContext->VSSetShader(m_VS, nullptr, 0);
			d3dContext->PSSetShader(m_PS, nullptr, 0);
		}

		// render mesh
		m_Mesh.render(d3dContext, rsCache, numInstances, 0, 1, 2);
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

void RenderableMesh::render(ID3D11DeviceContext* d3dContext, js::RenderStateCache* rsCache, size_t numInstances) const
{
	m_Impl.render(d3dContext, rsCache, numInstances);
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
// PostProcessor
//------------------------------------------------------------------------------
void PostProcessor::ConstBuffer_s::update(const CBaseCamera& camera)
{
	D3DXMATRIX scale(
		 2 / (float)DXUTGetDXGIBackBufferSurfaceDesc()->Width, 0, 0, 0,
		 0, -2 / (float)DXUTGetDXGIBackBufferSurfaceDesc()->Height, 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1);

	D3DXMATRIX bias(
		 1, 0, 0, 0,
		 0, 1, 0, 0,
		 0, 0, 1, 0,
		-1, 1, 0, 1);

	D3DXMATRIX viewProjectionMatrix = *camera.GetViewMatrix() * *camera.GetProjMatrix();
	D3DXMATRIX invViewProj;
	D3DXMatrixInverse(&invViewProj, nullptr, &viewProjectionMatrix);
	invViewProj = scale * bias * invViewProj;

	D3DXMatrixTranspose(&m_InvViewProjScaleBias, &invViewProj);

	m_ZParams.x = 1 / camera.GetFarClip() - 1 / camera.GetNearClip();
	m_ZParams.y = 1 / camera.GetNearClip();
	m_ZParams.z = camera.GetFarClip() - camera.GetNearClip();
	m_ZParams.w = camera.GetNearClip();
}

bool PostProcessor::valid() const
{
	return m_ScreenQuad.valid()
		&& m_PostVtxShd.valid()
		&& m_ConstBuf.valid()
		;
}

void PostProcessor::create(ID3D11Device* d3dDevice)
{
	m_PostVtxShd.createFromFile(d3dDevice, media(L"Common/Shader/Post.Vtx.hlsl"), "Main");
	js_assert(m_PostVtxShd.valid());

	m_ScreenQuad.create(d3dDevice, m_PostVtxShd.m_ByteCode);
	js_assert(m_ScreenQuad.valid());

	m_ConstBuf.create(d3dDevice);
}

void PostProcessor::destroy()
{
	m_PostVtxShd.destroy();
	m_ScreenQuad.destroy();
	m_ConstBuf.destroy();
}

void PostProcessor::filter(
	ID3D11DeviceContext* d3dContext,
	js::RenderStateCache& rsCache,
	js::RenderBuffer& srcBuf,
	js::RenderBuffer& dstBuf,
	js::PixelShader& shader
	)
{
	filter(d3dContext, rsCache, js::SrvVA() << srcBuf, js::RtvVA() << dstBuf, dstBuf.viewport(), shader);
}

void PostProcessor::filter(
	ID3D11DeviceContext* d3dContext,
	js::RenderStateCache& rsCache,
	js::SrvVA& srvVA,
	js::RenderBuffer& dstBuf,
	js::PixelShader& shader
	)
{
	filter(d3dContext, rsCache, srvVA, js::RtvVA() << dstBuf, dstBuf.viewport(), shader);
}

void PostProcessor::filter(
	ID3D11DeviceContext* d3dContext,
	js::RenderStateCache& rsCache,
	js::SrvVA& srvVA,
	js::RtvVA& rtvVA,
	const D3D11_VIEWPORT& vp,
	js::PixelShader& shader
	)
{
	if(rtvVA.m_Count > 0)
	{
		rsCache.rtState().backup();
		rsCache.rtState().set(rtvVA.m_Count, rtvVA, nullptr);
	}

	size_t vpCnt = 0; D3D11_VIEWPORT vps[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	d3dContext->RSGetViewports(&vpCnt, nullptr);
	d3dContext->RSGetViewports(&vpCnt, vps);
	d3dContext->RSSetViewports(1, js::VpVA() << vp);

	filter(d3dContext, rsCache, srvVA, shader);
	
	d3dContext->RSSetViewports(vpCnt, vps);

	if(rtvVA.m_Count > 0)
	{
		rsCache.rtState().restore();
	}
}

void PostProcessor::filter(
	ID3D11DeviceContext* d3dContext,
	js::RenderStateCache& rsCache,
	js::SrvVA& srvVA,
	js::PixelShader& shader
	)
{
	rsCache.vsState().backup();
	rsCache.vsState().setShader(m_PostVtxShd);

	rsCache.psState().backup();
	rsCache.psState().setShader(shader);
	rsCache.psState().setSRViews(0, srvVA.m_Count, srvVA);
	rsCache.psState().setConstBuffers(0, 1, js::BufVA() << m_ConstBuf);

	rsCache.samplerState().AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	rsCache.samplerState().AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	rsCache.samplerState().AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	rsCache.samplerState().Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	rsCache.samplerState().dirty();
	rsCache.psState().setSamplers(0, 1, js::SampVA() << *rsCache.samplerState().current());
	
	rsCache.applyToContext(d3dContext);

	m_ScreenQuad.render(d3dContext);
	
	rsCache.vsState().restore();
	rsCache.psState().restore();
}

//------------------------------------------------------------------------------
// SceneShaderConstants
//------------------------------------------------------------------------------
void SceneShaderConstants::updateCameraContants(const CBaseCamera& camera)
{
	D3DXMATRIX viewProjectionMatrix = *camera.GetViewMatrix() * *camera.GetProjMatrix();
	D3DXMatrixTranspose(&m_ViewProjection, &viewProjectionMatrix);
	
	m_CameraPosition.x = camera.GetEyePt()->x;
	m_CameraPosition.y = camera.GetEyePt()->y;
	m_CameraPosition.z = camera.GetEyePt()->z;
	m_CameraPosition.w = 1;

	m_CameraParams.x = camera.GetNearClip();
	m_CameraParams.y = camera.GetFarClip() - camera.GetNearClip();
	m_CameraParams.z = 0;
	m_CameraParams.w = 0;
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
    DXUTInit( true, true, L"" ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( app.getName() );

    // Only require 10-level hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 800, 450 );
    DXUTMainLoop(); // Enter into the DXUT ren  der loop

    // Perform any application-level cleanup here
	int exitCode = DXUTGetExitCode();
	DXUTDestroyState();

    return exitCode;
}

DXUTApp::DXUTApp() : m_GuiDlgResMgr(0), m_GuiTxtHelper(0)
{
	m_GuiDlgResMgr = new CDXUTDialogResourceManager;
}

DXUTApp::~DXUTApp()
{
	for(size_t i=0; i<m_GuiDlgs.size(); ++i)
		delete m_GuiDlgs[i];

	if(m_GuiDlgResMgr)
	{
		delete m_GuiDlgResMgr;
		m_GuiDlgResMgr = nullptr;
	}
}

void DXUTApp::guiOnD3D11CreateDevice(ID3D11Device* d3dDevice)
{
	ID3D11DeviceContext* d3dContext = DXUTGetD3D11DeviceContext();

	m_GuiDlgResMgr->OnD3D11CreateDevice(d3dDevice, d3dContext);
	m_GuiTxtHelper = new CDXUTTextHelper(d3dDevice, d3dContext, m_GuiDlgResMgr, 15);
}

void DXUTApp::guiOnD3D11DestroyDevice()
{
	if(m_GuiTxtHelper)
	{
		delete m_GuiTxtHelper;
		m_GuiTxtHelper = nullptr;
	}
	
	m_GuiDlgResMgr->OnD3D11DestroyDevice();
}

void DXUTApp::guiOnD3D11ResizedSwapChain(ID3D11Device* d3dDevice, const DXGI_SURFACE_DESC* backBufferSurfaceDesc)
{
	if(!m_GuiDlgResMgr) return;
	m_GuiDlgResMgr->OnD3D11ResizedSwapChain(d3dDevice, backBufferSurfaceDesc);
}

void DXUTApp::guiOnD3D11ReleasingSwapChain()
{
	if(!m_GuiDlgResMgr) return;
	m_GuiDlgResMgr->OnD3D11ReleasingSwapChain();
}

int DXUTApp::guiMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing)
{
	if(!m_GuiDlgResMgr) return 0;

	*pbNoFurtherProcessing = m_GuiDlgResMgr->MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

	for(size_t i=0; i<m_GuiDlgs.size(); ++i)
	{
		*pbNoFurtherProcessing = m_GuiDlgs[i]->MsgProc( hWnd, uMsg, wParam, lParam );
		if( *pbNoFurtherProcessing )
			return 0;
	}

	return 0;
}