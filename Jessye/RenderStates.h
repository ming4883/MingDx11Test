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
	void create(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateBlendState(this, &m_StateObject);
	}
};	// BlendState

struct DepthStencilState : public RenderState_t<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState>
{
	void create(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateDepthStencilState(this, &m_StateObject);
	}
};	// DepthStencilState

struct RasterizerState : public RenderState_t<D3D11_RASTERIZER_DESC, ID3D11RasterizerState>
{
	void create(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateRasterizerState(this, &m_StateObject);
	}
};	// RasterizerState

struct SamplerState : public RenderState_t<D3D11_SAMPLER_DESC, ID3D11SamplerState>
{
	void create(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateSamplerState(this, &m_StateObject);
	}
};	// SamplerState

class BlendStateCache : public BlendState
{
public:
	BlendStateCache(ID3D11Device* d3dDevice);
	~BlendStateCache();

	void dirty();
	void backup();
	void restore();
	BlendState* current();

private:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(BlendStateCache);

};	// BlendStateCache

class DepthStencilStateCache : public DepthStencilState
{
public:
	DepthStencilStateCache(ID3D11Device* d3dDevice);
	~DepthStencilStateCache();

	void dirty();
	void backup();
	void restore();
	DepthStencilState* current();

private:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(DepthStencilStateCache);

};	// DepthStencilStateCache

class RasterizerStateCache : public RasterizerState
{
public:
	RasterizerStateCache(ID3D11Device* d3dDevice);
	~RasterizerStateCache();

	void dirty();
	void backup();
	void restore();
	RasterizerState* current();

private:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(RasterizerStateCache);

};	// RasterizerStateCache

class RenderTargetStateCache
{
public:
	RenderTargetStateCache();
	~RenderTargetStateCache();

	void set(size_t count, ID3D11RenderTargetView* const * rtv, ID3D11DepthStencilView *dsv);
	void backup();
	void restore();
	ID3D11RenderTargetView** currentRTV();
	ID3D11DepthStencilView* currentDSV();

private:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(RenderTargetStateCache);
};

class RenderStateCache
{
public:
	RenderStateCache();
	~RenderStateCache();

	void create(ID3D11Device* d3dDevice);
	void destroy();

	BlendStateCache& blendState();

	DepthStencilStateCache& depthStencilState();

	RasterizerStateCache& rasterizerState();

	RenderTargetStateCache& renderTargetState();

	void applyToContext(ID3D11DeviceContext* d3dContext);

private:
	class Impl;
	Impl& m_Impl;
	js_decl_non_copyable(RenderStateCache);

};	// RenderStateCache


}	// namespace js

#endif	// RENDERSTATES_H