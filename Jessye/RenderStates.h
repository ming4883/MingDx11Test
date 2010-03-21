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

template<typename T>
struct RenderState_t
{
	T* m_StateObject;

	RenderState_t() : m_StateObject(nullptr) {}

	bool valid() const { return nullptr != m_StateObject;}

	void destroy() { js_safe_release(m_StateObject); }

	virtual ~RenderState_t() { destroy(); }

	operator T* () { return m_StateObject; }
};


struct BlendState : public D3D11_BLEND_DESC, public RenderState_t<ID3D11BlendState>
{
	BlendState()
	{
		RenderStates::init(*this);
	}

	void create(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateBlendState(this, &m_StateObject);
	}
};

struct DepthStencilState : public D3D11_DEPTH_STENCIL_DESC, public RenderState_t<ID3D11DepthStencilState>
{
	DepthStencilState()
	{
		RenderStates::init(*this);
	}

	void create(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateDepthStencilState(this, &m_StateObject);
	}
};

struct RasterizerState : public D3D11_RASTERIZER_DESC, public RenderState_t<ID3D11RasterizerState>
{
	RasterizerState()
	{
		RenderStates::init(*this);
	}

	void create(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateRasterizerState(this, &m_StateObject);
	}
};

struct SamplerState : public D3D11_SAMPLER_DESC, public RenderState_t<ID3D11SamplerState>
{
	SamplerState()
	{
		RenderStates::init(*this);
	}

	void create(ID3D11Device* d3dDevice)
	{
		destroy();
		d3dDevice->CreateSamplerState(this, &m_StateObject);
	}
};


}	// namespace js

#endif	// RENDERSTATES_H