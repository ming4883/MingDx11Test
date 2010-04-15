#ifndef RENDERSTATES_H
#define RENDERSTATES_H

#include "Platform.h"

namespace js
{

struct RenderStates
{
	static void init(D3D11_DEPTH_STENCIL_DESC& desc);

	static void init(D3D11_BLEND_DESC& desc);

	static void init(D3D11_RASTERIZER_DESC& desc);

	static void init(D3D11_SAMPLER_DESC& desc);
};	// RenderStates

template<typename Desc, typename T>
struct RenderState_t : public Desc
{
	T* m_StateObject;

	RenderState_t() : m_StateObject(nullptr) {RenderStates::init(*this);}

	bool valid() const { return nullptr != m_StateObject;}

	void destroy() { js_safe_release(m_StateObject); }

	virtual ~RenderState_t() { destroy(); }

	operator T* () { return m_StateObject; }

	void copyDesc(const RenderState_t& rvalue)
	{
		(Desc&)(*this) = (const Desc&)rvalue;
	}
};	// RenderState_t

struct BlendState : public RenderState_t<D3D11_BLEND_DESC, ID3D11BlendState>
{
	void createStateObject(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateBlendState(this, &m_StateObject);
	}
};	// BlendState

struct DepthStencilState : public RenderState_t<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState>
{
	void createStateObject(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateDepthStencilState(this, &m_StateObject);
	}
};	// DepthStencilState

struct RasterizerState : public RenderState_t<D3D11_RASTERIZER_DESC, ID3D11RasterizerState>
{
	void createStateObject(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateRasterizerState(this, &m_StateObject);
	}
};	// RasterizerState

struct SamplerState : public RenderState_t<D3D11_SAMPLER_DESC, ID3D11SamplerState>
{
	void createStateObject(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateSamplerState(this, &m_StateObject);
	}
};	// SamplerState

class BlendStateCache : public BlendState
{
public:
	BlendStateCache();
	~BlendStateCache();

	void dirty();
	void backup();
	void restore();
	BlendState* current();
	void create(ID3D11Device* d3dDevice);
	void destroy();
	void applyToContext(ID3D11DeviceContext* d3dContext);

	float* blendFactor();

private:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(BlendStateCache);

};	// BlendStateCache

class DepthStencilStateCache : public DepthStencilState
{
public:
	DepthStencilStateCache();
	~DepthStencilStateCache();

	void dirty();
	void backup();
	void restore();
	DepthStencilState* current();
	void create(ID3D11Device* d3dDevice);
	void destroy();
	void applyToContext(ID3D11DeviceContext* d3dContext);

private:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(DepthStencilStateCache);

};	// DepthStencilStateCache

class RasterizerStateCache : public RasterizerState
{
public:
	RasterizerStateCache();
	~RasterizerStateCache();

	void dirty();
	void backup();
	void restore();
	RasterizerState* current();
	void create(ID3D11Device* d3dDevice);
	void destroy();
	void applyToContext(ID3D11DeviceContext* d3dContext);

private:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(RasterizerStateCache);

};	// RasterizerStateCache

class SamplerStateCache
{
public:
	SamplerStateCache();
	~SamplerStateCache();

	void create(ID3D11Device* d3dDevice);
	void destroy();

	SamplerState* get(const SamplerState& prototype);

private:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(SamplerStateCache);

};	// SamplerStateCache

class ShaderStateCache
{
public:
	ShaderStateCache();
	virtual ~ShaderStateCache();

	void create(ID3D11Device* d3dDevice);
	void destroy();

	void backup();
	void restore();

	void setConstBuffers(size_t startSlot, size_t count, ID3D11Buffer* const * buffers);
	void setSRViews(size_t startSlot, size_t count, ID3D11ShaderResourceView* const * srViews);
	void setSamplers(size_t startSlot, size_t count, ID3D11SamplerState *const *samplers);

	ID3D11Buffer** currentConstBuffers();
	ID3D11ShaderResourceView** currentSRViews();
	ID3D11SamplerState** currentSamplers();
	
	// D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT 
	size_t maxNumConstBuffers() const;
	// D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT 
	size_t maxNumSRViews() const;
	// D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT
	size_t maxNumSamplers() const;

protected:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(ShaderStateCache);
};	// ShaderStateCache

class VertexShaderStateCache : public ShaderStateCache
{
public:
	void setShader(ID3D11VertexShader* shader);
	ID3D11VertexShader* currentShader();
	void applyToContext(ID3D11DeviceContext* d3dContext);
	
};	// VertexShaderStateCache

class GeometryShaderStateCache : public ShaderStateCache
{
public:
	void setShader(ID3D11GeometryShader* shader);
	ID3D11GeometryShader* currentShader();
	void applyToContext(ID3D11DeviceContext* d3dContext);
	
};	// GeometryShaderStateCache

class PixelShaderStateCache : public ShaderStateCache
{
public:
	void setShader(ID3D11PixelShader* shader);
	ID3D11PixelShader* currentShader();
	void applyToContext(ID3D11DeviceContext* d3dContext);
	
};	// PixelShaderStateCache

class RenderTargetStateCache
{
public:
	RenderTargetStateCache();
	~RenderTargetStateCache();

	void create(ID3D11Device* d3dDevice);
	void destroy();

	void backup();
	void restore();

	void preApplyToContext(ID3D11DeviceContext* d3dContext);
	void postApplyToContext(ID3D11DeviceContext* d3dContext);

	void set(size_t count, ID3D11RenderTargetView* const * rtViews, ID3D11DepthStencilView *dsView);
	
	ID3D11RenderTargetView** currentRTView();
	ID3D11DepthStencilView* currentDSView();

	// D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT
	size_t maxNumRTViews() const;

private:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(RenderTargetStateCache);
};	// RenderTargetStateCache


class RenderStateCache
	: protected BlendStateCache
	, protected DepthStencilStateCache
	, protected RasterizerStateCache
	, protected SamplerStateCache
	, protected VertexShaderStateCache
	, protected GeometryShaderStateCache
	, protected PixelShaderStateCache
	, protected RenderTargetStateCache
{
public:
	BlendStateCache& blendState() { return *this; }

	DepthStencilStateCache& depthStencilState() { return *this; }

	RasterizerStateCache& rasterizerState() { return *this; }

	SamplerStateCache& samplerState() { return *this; }

	VertexShaderStateCache& vsState() { return *this; }

	GeometryShaderStateCache& gsState() { return *this; }

	PixelShaderStateCache& psState() { return *this; }
	
	RenderTargetStateCache& rtState() { return *this; }
	
	void create(ID3D11Device* d3dDevice)
	{
		BlendStateCache::create(d3dDevice);
		DepthStencilStateCache::create(d3dDevice);
		RasterizerStateCache::create(d3dDevice);
		SamplerStateCache::create(d3dDevice);
		VertexShaderStateCache::create(d3dDevice);
		GeometryShaderStateCache::create(d3dDevice);
		PixelShaderStateCache::create(d3dDevice);
		RenderTargetStateCache::create(d3dDevice);
	}

	void destroy()
	{
		BlendStateCache::destroy();
		DepthStencilStateCache::destroy();
		RasterizerStateCache::destroy();
		SamplerStateCache::destroy();
		VertexShaderStateCache::destroy();
		GeometryShaderStateCache::destroy();
		PixelShaderStateCache::destroy();
		RenderTargetStateCache::destroy();
	}

	void applyToContext(ID3D11DeviceContext* d3dContext)
	{
		RenderTargetStateCache::preApplyToContext(d3dContext);

		BlendStateCache::applyToContext(d3dContext);
		DepthStencilStateCache::applyToContext(d3dContext);
		RasterizerStateCache::applyToContext(d3dContext);
		VertexShaderStateCache::applyToContext(d3dContext);
		GeometryShaderStateCache::applyToContext(d3dContext);
		PixelShaderStateCache::applyToContext(d3dContext);
		
		RenderTargetStateCache::postApplyToContext(d3dContext);
	}

};	// RenderStateCache


}	// namespace js

#endif	// RENDERSTATES_H