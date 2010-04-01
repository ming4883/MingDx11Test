#include "DXUT.h"
#include "RenderStates.h"
#include <map>

namespace js
{
	void RenderStates::init(D3D11_DEPTH_STENCIL_DESC& desc)
	{
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.StencilEnable = FALSE;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_READ_MASK;

		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

		desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	}

	void RenderStates::init(D3D11_BLEND_DESC& desc)
	{
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;
		
		for(int i=0; i<8; ++i)
		{
			desc.RenderTarget[i].BlendEnable = FALSE;
			desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
			desc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
			desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
	}

	void RenderStates::init(D3D11_RASTERIZER_DESC& desc)
	{
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_BACK;
		desc.FrontCounterClockwise = FALSE;
		desc.DepthBias = 0;
		desc.SlopeScaledDepthBias = 0;
		desc.DepthBiasClamp = 0;
		desc.DepthClipEnable = TRUE;
		desc.ScissorEnable = FALSE;
		desc.MultisampleEnable = FALSE;
		desc.AntialiasedLineEnable = FALSE;
	}

	void RenderStates::init(D3D11_SAMPLER_DESC& desc)
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.MinLOD = -FLT_MAX;
		desc.MaxLOD = FLT_MAX;
		desc.MipLODBias = 0;
		desc.MaxAnisotropy = 16;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.BorderColor[0] = 0;
		desc.BorderColor[1] = 0;
		desc.BorderColor[2] = 0;
		desc.BorderColor[3] = 0;
	}

	//--------------------------------------------------------------------------
	template<typename T>
	struct Cache_t : protected std::map<size_t, T*>
	{
		typedef std::map<size_t, T*> BaseClass;
		typedef BaseClass::iterator Iter;

		ID3D11Device* m_D3dDevice;
		T* m_Prototype;
		bool m_PrototypeDirty;
		T* m_Current;
		T* m_Backup;

		Cache_t(ID3D11Device* d3dDevice)
			: m_D3dDevice(d3dDevice)
			, m_Current(nullptr)
			, m_Prototype(nullptr)
			, m_PrototypeDirty(true)
			, m_Backup(nullptr)
		{
		}

		virtual ~Cache_t()
		{
			for(Iter iter = begin(); iter != end(); ++iter)
				delete iter->second;
		}

		static size_t hashCode(T& t)
		{
			const char* buf = (const char*)&t;
			const size_t len = sizeof(T);

			size_t hash = 0;
		    
			for(size_t i=0; i<len; ++i) {
				//hash = hash * 65599 + buf[i];
				hash = buf[i] + (hash << 6) + (hash << 16) - hash;
			}

			return hash;
		}

		T* current()
		{
			js_assert(nullptr != m_Prototype);
			if(m_PrototypeDirty)
			{
				size_t hash = hashCode(*m_Prototype);
				Iter it = find(hash);

				if(end() == it)
				{
					T* state = new T;
					state->copyDesc(*m_Prototype);
					state->create(m_D3dDevice);
					js_assert(state->valid());

					insert(std::make_pair(hash, state));

					it = find(hash);
					js_assert(it != end());
				}

				m_Current = it->second;

				// reset dirty flag
				m_PrototypeDirty = false;
			}

			js_assert(nullptr != m_Current);
			return m_Current;
		}

		void backup()
		{
			js_assert(nullptr == m_Backup);
			m_Backup = current();
		}

		void restore()
		{
			js_assert(nullptr != m_Backup);

			m_Current = m_Backup;
			m_Prototype->copyDesc(*m_Backup);

			m_Backup = nullptr;

		}
	};	// Cache_t

	//--------------------------------------------------------------------------
	class BlendStateCache::Impl : public Cache_t<BlendState>
	{
		friend class BlendStateCache;
		Impl(ID3D11Device* d3dDevice) : Cache_t<BlendState>(d3dDevice) {}
	};

	BlendStateCache::BlendStateCache(ID3D11Device* d3dDevice)
		: m_Impl(*new Impl(d3dDevice))
	{
		m_Impl.m_Prototype = this;
	}

	BlendStateCache::~BlendStateCache()
	{
		delete &m_Impl;
	}

	void BlendStateCache::dirty()
	{
		m_Impl.m_PrototypeDirty = true;
	}

	void BlendStateCache::backup()
	{
		m_Impl.backup();
	}

	void BlendStateCache::restore()
	{
		m_Impl.restore();
	}

	BlendState* BlendStateCache::current()
	{
		return m_Impl.current();
	}

	//--------------------------------------------------------------------------
	class DepthStencilStateCache::Impl : public Cache_t<DepthStencilState>
	{
		friend class DepthStencilStateCache;
		Impl(ID3D11Device* d3dDevice) : Cache_t<DepthStencilState>(d3dDevice) {}
	};

	DepthStencilStateCache::DepthStencilStateCache(ID3D11Device* d3dDevice)
		: m_Impl(*new Impl(d3dDevice))
	{
		m_Impl.m_Prototype = this;
	}

	DepthStencilStateCache::~DepthStencilStateCache()
	{
		delete &m_Impl;
	}

	void DepthStencilStateCache::dirty()
	{
		m_Impl.m_PrototypeDirty = true;
	}

	void DepthStencilStateCache::backup()
	{
		m_Impl.backup();
	}

	void DepthStencilStateCache::restore()
	{
		m_Impl.restore();
	}

	DepthStencilState* DepthStencilStateCache::current()
	{
		return m_Impl.current();
	}

	//--------------------------------------------------------------------------
	class RasterizerStateCache::Impl : public Cache_t<RasterizerState>
	{
		friend class RasterizerStateCache;
		Impl(ID3D11Device* d3dDevice) : Cache_t<RasterizerState>(d3dDevice) {}
	};

	RasterizerStateCache::RasterizerStateCache(ID3D11Device* d3dDevice)
		: m_Impl(*new Impl(d3dDevice))
	{
		m_Impl.m_Prototype = this;
	}

	RasterizerStateCache::~RasterizerStateCache()
	{
		delete &m_Impl;
	}

	void RasterizerStateCache::dirty()
	{
		m_Impl.m_PrototypeDirty = true;
	}

	void RasterizerStateCache::backup()
	{
		m_Impl.backup();
	}

	void RasterizerStateCache::restore()
	{
		m_Impl.restore();
	}

	RasterizerState* RasterizerStateCache::current()
	{
		return m_Impl.current();
	}
	
	//--------------------------------------------------------------------------
	class RenderTargetStateCache::Impl
	{
		friend class RenderTargetStateCache;

		enum {MAX_RTV = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT};

		struct State
		{
			ID3D11RenderTargetView* m_RTV[MAX_RTV];
			ID3D11DepthStencilView* m_DSV;
		};

		State m_Current;
		State m_Backup;
	};

	RenderTargetStateCache::RenderTargetStateCache()
		: m_Impl(*new Impl)
	{
	}

	RenderTargetStateCache::~RenderTargetStateCache()
	{
		delete &m_Impl;
	}

	void RenderTargetStateCache::set(size_t count, ID3D11RenderTargetView* const * rtv, ID3D11DepthStencilView *dsv)
	{
		for(size_t i=0; i<count; ++i)
			m_Impl.m_Current.m_RTV[i] = rtv[i];

		for(size_t i=count; i<Impl::MAX_RTV; ++i)
			m_Impl.m_Current.m_RTV[i] = nullptr;

		m_Impl.m_Current.m_DSV = dsv;
	}

	void RenderTargetStateCache::backup()
	{
		m_Impl.m_Backup = m_Impl.m_Current;
	}

	void RenderTargetStateCache::restore()
	{
		m_Impl.m_Current = m_Impl.m_Backup;
	}

	ID3D11RenderTargetView** RenderTargetStateCache::currentRTV()
	{
		return m_Impl.m_Current.m_RTV;
	}

	ID3D11DepthStencilView* RenderTargetStateCache::currentDSV()
	{
		return m_Impl.m_Current.m_DSV;
	}

	//--------------------------------------------------------------------------
	class RenderStateCache::Impl
	{
		friend class RenderStateCache;

		BlendStateCache* m_BlendStateCache;
		DepthStencilStateCache* m_DepthStencilStateCache;
		RasterizerStateCache* m_RasterizerStateCache;
		RenderTargetStateCache* m_RenderTargetStateCache;

		float m_BlendFactor[4];
		unsigned int m_StencilRef;

		Impl()
			: m_BlendStateCache(nullptr)
			, m_DepthStencilStateCache(nullptr)
			, m_RasterizerStateCache(nullptr)
			, m_RenderTargetStateCache(nullptr)
		{
			m_BlendFactor[0] = m_BlendFactor[1] = m_BlendFactor[2] = m_BlendFactor[3] = 0;

			m_StencilRef = 0;
		}

		~Impl()
		{
			destroy();
		}

		void create(ID3D11Device* d3dDevice)
		{
			m_BlendStateCache = new BlendStateCache(d3dDevice);
			m_DepthStencilStateCache = new DepthStencilStateCache(d3dDevice);
			m_RasterizerStateCache = new RasterizerStateCache(d3dDevice);
			m_RenderTargetStateCache = new RenderTargetStateCache;
		}

		void destroy()
		{
			delete m_BlendStateCache;
			m_BlendStateCache  = nullptr;

			delete m_DepthStencilStateCache;
			m_DepthStencilStateCache = nullptr;

			delete m_RasterizerStateCache;
			m_RasterizerStateCache = nullptr;

			delete m_RenderTargetStateCache;
			m_RenderTargetStateCache = nullptr;
		}

	};	// RenderStateCache::Impl

	RenderStateCache::RenderStateCache()
		: m_Impl(*new Impl)
	{
	}

	RenderStateCache::~RenderStateCache()
	{
		delete &m_Impl;
	}

	void RenderStateCache::create(ID3D11Device* d3dDevice)
	{
		m_Impl.create(d3dDevice);
	}

	void RenderStateCache::destroy()
	{
		m_Impl.destroy();
	}

	BlendStateCache& RenderStateCache::blendState()
	{
		return *m_Impl.m_BlendStateCache;
	}

	DepthStencilStateCache& RenderStateCache::depthStencilState()
	{
		return *m_Impl.m_DepthStencilStateCache;
	}

	RasterizerStateCache& RenderStateCache::rasterizerState()
	{
		return *m_Impl.m_RasterizerStateCache;
	}

	RenderTargetStateCache& RenderStateCache::renderTargetState()
	{
		return *m_Impl.m_RenderTargetStateCache;
	}

	void RenderStateCache::applyToContext(ID3D11DeviceContext* d3dContext)
	{
		d3dContext->OMSetBlendState(*m_Impl.m_BlendStateCache->current(), m_Impl.m_BlendFactor, 0xffffffff);

		d3dContext->OMSetDepthStencilState(*m_Impl.m_DepthStencilStateCache->current(), m_Impl.m_StencilRef);

		d3dContext->RSSetState(*m_Impl.m_RasterizerStateCache->current());
	}

}	// namespace js