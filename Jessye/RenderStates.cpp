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
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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
		
		Cache_t()
			: m_D3dDevice(nullptr)
		{
		}

		virtual ~Cache_t()
		{
			destroy();
		}

		void create(ID3D11Device* d3dDevice)
		{
			m_D3dDevice = d3dDevice;
		}

		void destroy()
		{
			for(Iter iter = begin(); iter != end(); ++iter)
				delete iter->second;
			clear();
		}

		static size_t hashCode(const T& t)
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

		T* get(const T& prototoype)
		{
			size_t hash = hashCode(prototoype);
			Iter it = find(hash);

			if(end() == it)
			{
				T* state = new T;
				state->copyDesc(prototoype);
				state->createStateObject(m_D3dDevice);
				js_assert(state->valid());

				insert(std::make_pair(hash, state));

				it = find(hash);
				js_assert(it != end());
			}

			return it->second;
		}

	};	// Cache_t

	template<typename T>
	struct CacheWithPrototype_t : public Cache_t<T>
	{
		T* m_Prototype;
		bool m_PrototypeDirty;
		T* m_Current;
		T* m_Backup;

		CacheWithPrototype_t()
			: m_Current(nullptr)
			, m_Backup(nullptr)
			, m_Prototype(nullptr)
			, m_PrototypeDirty(true)
		{
		}

		virtual ~CacheWithPrototype_t()
		{
		}

		void create(ID3D11Device* d3dDevice)
		{
			Cache_t::create(d3dDevice);
			m_Current = nullptr;
			m_PrototypeDirty = true;
			m_Backup = nullptr;
		}

		T* current()
		{
			js_assert(nullptr != m_Prototype);
			if(m_PrototypeDirty)
			{
				m_Current = get(*m_Prototype);

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

			m_PrototypeDirty = true;
		}
	};	// CacheWithPrototype_t

	//--------------------------------------------------------------------------
	class BlendStateCache::Impl : public CacheWithPrototype_t<BlendState>
	{
		friend class BlendStateCache;
		
		float m_BlendFactor[4];
		float m_BlendFactorBackup[4];
		
		Impl()
		{
			m_BlendFactor[0] = m_BlendFactor[1] = m_BlendFactor[2] = m_BlendFactor[3] = 0;
		}
	};

	BlendStateCache::BlendStateCache()
		: m_Impl(*new Impl)
	{
		m_Impl.m_Prototype = this;
	}

	BlendStateCache::~BlendStateCache()
	{
		delete &m_Impl;
	}

	void BlendStateCache::create(ID3D11Device* d3dDevice)
	{
		m_Impl.create(d3dDevice);
	}

	void BlendStateCache::destroy()
	{
		m_Impl.destroy();
	}

	void BlendStateCache::dirty()
	{
		m_Impl.m_PrototypeDirty = true;
	}

	void BlendStateCache::backup()
	{
		m_Impl.backup();

		for(size_t i=0; i<4; ++i)
			m_Impl.m_BlendFactorBackup[i] = m_Impl.m_BlendFactor[i];
	}

	void BlendStateCache::restore()
	{
		m_Impl.restore();

		for(size_t i=0; i<4; ++i)
			m_Impl.m_BlendFactor[i] = m_Impl.m_BlendFactorBackup[i];
	}

	BlendState* BlendStateCache::current()
	{
		return m_Impl.current();
	}

	void BlendStateCache::applyToContext(ID3D11DeviceContext* d3dContext)
	{
		if(!m_Impl.m_PrototypeDirty)
			return;

		d3dContext->OMSetBlendState(*m_Impl.current(), m_Impl.m_BlendFactor, 0xffffffff);
	}

	float* BlendStateCache::blendFactor()
	{
		return m_Impl.m_BlendFactor;
	}

	//--------------------------------------------------------------------------
	class DepthStencilStateCache::Impl : public CacheWithPrototype_t<DepthStencilState>
	{
		friend class DepthStencilStateCache;
		
		unsigned int m_StencilRef;
		
		Impl()
		{
			m_StencilRef = 0;
		}
	};

	DepthStencilStateCache::DepthStencilStateCache()
		: m_Impl(*new Impl)
	{
		m_Impl.m_Prototype = this;
	}

	DepthStencilStateCache::~DepthStencilStateCache()
	{
		delete &m_Impl;
	}

	void DepthStencilStateCache::create(ID3D11Device* d3dDevice)
	{
		m_Impl.create(d3dDevice);
	}

	void DepthStencilStateCache::destroy()
	{
		m_Impl.destroy();
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

	void DepthStencilStateCache::applyToContext(ID3D11DeviceContext* d3dContext)
	{
		if(!m_Impl.m_PrototypeDirty)
			return;

		d3dContext->OMSetDepthStencilState(*m_Impl.current(), m_Impl.m_StencilRef);
	}

	//--------------------------------------------------------------------------
	class RasterizerStateCache::Impl : public CacheWithPrototype_t<RasterizerState>
	{
		friend class RasterizerStateCache;
		Impl() {}
	};

	RasterizerStateCache::RasterizerStateCache()
		: m_Impl(*new Impl)
	{
		m_Impl.m_Prototype = this;
	}

	RasterizerStateCache::~RasterizerStateCache()
	{
		delete &m_Impl;
	}

	void RasterizerStateCache::create(ID3D11Device* d3dDevice)
	{
		m_Impl.create(d3dDevice);
	}

	void RasterizerStateCache::destroy()
	{
		m_Impl.destroy();
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
	
	void RasterizerStateCache::applyToContext(ID3D11DeviceContext* d3dContext)
	{
		if(!m_Impl.m_PrototypeDirty)
			return;

		d3dContext->RSSetState(*m_Impl.current());
	}

	//--------------------------------------------------------------------------
	class SamplerStateCache::Impl : public CacheWithPrototype_t<SamplerState>
	{
		friend class SamplerStateCache;
		Impl() {}
	};

	SamplerStateCache::SamplerStateCache()
		: m_Impl(*new Impl)
	{
		m_Impl.m_Prototype = this;
	}

	SamplerStateCache::~SamplerStateCache()
	{
		delete &m_Impl;
	}

	void SamplerStateCache::create(ID3D11Device* d3dDevice)
	{
		m_Impl.create(d3dDevice);
	}

	void SamplerStateCache::destroy()
	{
		m_Impl.destroy();
	}

	void SamplerStateCache::dirty()
	{
		m_Impl.m_PrototypeDirty = true;
	}

	void SamplerStateCache::backup()
	{
		m_Impl.backup();
	}

	void SamplerStateCache::restore()
	{
		m_Impl.restore();
	}

	SamplerState* SamplerStateCache::current()
	{
		return m_Impl.current();
	}

	//--------------------------------------------------------------------------
	class ShaderStateCache::Impl
	{
		friend class ShaderStateCache;
		friend class VertexShaderStateCache;
		friend class GeometryShaderStateCache;
		friend class PixelShaderStateCache;

		enum
		{
			MAX_CONSTBUFFER = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT,
			MAX_SRVIEW = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
			MAX_SAMPLER = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT,
		};

#pragma pack(push)
#pragma pack(1)
		struct State
		{
			void* m_Shader;
			ID3D11Buffer* m_ConstBuffers[MAX_CONSTBUFFER];
			ID3D11ShaderResourceView* m_SRViews[MAX_SRVIEW];
			ID3D11SamplerState* m_Samplers[MAX_SAMPLER];

			State() {memset(this, 0, sizeof(State));}
		};
#pragma pack(pop)


		State m_Current;
		State m_Backup;
		bool m_Dirty;
	};

	ShaderStateCache::ShaderStateCache()
		: m_Impl(*new Impl)
	{
		m_Impl.m_Dirty = true;
	}

	ShaderStateCache::~ShaderStateCache()
	{
		delete &m_Impl;
	}

	void ShaderStateCache::create(ID3D11Device* d3dDevice)
	{
		m_Impl.m_Current = Impl::State();
		m_Impl.m_Backup = Impl::State();
		m_Impl.m_Dirty = true;
	}

	void ShaderStateCache::destroy()
	{
	}

	void ShaderStateCache::backup()
	{
		m_Impl.m_Backup = m_Impl.m_Current;
	}

	void ShaderStateCache::restore()
	{
		m_Impl.m_Current = m_Impl.m_Backup;
		m_Impl.m_Dirty = true;
	}

	void ShaderStateCache::setConstBuffers(size_t startSlot, size_t count, ID3D11Buffer* const * buffers)
	{
		size_t maxSlot = startSlot + count;
		if(maxSlot > this->maxNumConstBuffers())
			maxSlot = this->maxNumConstBuffers();

		for(size_t i=startSlot; i<maxSlot; ++i)
			m_Impl.m_Current.m_ConstBuffers[i] = buffers[i-startSlot];
		
		m_Impl.m_Dirty = true;
	}
	
	void ShaderStateCache::setSRViews(size_t startSlot, size_t count, ID3D11ShaderResourceView* const * srViews)
	{
		size_t maxSlot = startSlot + count;
		if(maxSlot > this->maxNumSRViews())
			maxSlot = this->maxNumSRViews();

		for(size_t i=startSlot; i<maxSlot; ++i)
			m_Impl.m_Current.m_SRViews[i] = srViews[i-startSlot];

		m_Impl.m_Dirty = true;
	}

	void ShaderStateCache::setSamplers(size_t startSlot, size_t count, ID3D11SamplerState *const *samplers)
	{
		size_t maxSlot = startSlot + count;
		if(maxSlot > this->maxNumSamplers())
			maxSlot = this->maxNumSamplers();

		for(size_t i=startSlot; i<maxSlot; ++i)
			m_Impl.m_Current.m_Samplers[i] = samplers[i-startSlot];

		m_Impl.m_Dirty = true;
	}

	ID3D11Buffer** ShaderStateCache::currentConstBuffers()
	{
		return m_Impl.m_Current.m_ConstBuffers;
	}

	ID3D11ShaderResourceView** ShaderStateCache::currentSRViews()
	{
		return m_Impl.m_Current.m_SRViews;
	}

	ID3D11SamplerState** ShaderStateCache::currentSamplers()
	{
		return m_Impl.m_Current.m_Samplers;
	}
	
	size_t ShaderStateCache::maxNumConstBuffers() const
	{
		return Impl::MAX_CONSTBUFFER;
	}

	size_t ShaderStateCache::maxNumSRViews() const
	{
		return Impl::MAX_SRVIEW;
	}

	size_t ShaderStateCache::maxNumSamplers() const
	{
		return Impl::MAX_SAMPLER;
	}

	/** Concrete ShaderStateCache */
	void VertexShaderStateCache::setShader(ID3D11VertexShader* shader)
	{
		m_Impl.m_Current.m_Shader = shader;
		m_Impl.m_Dirty = true;
	}

	ID3D11VertexShader* VertexShaderStateCache::currentShader()
	{
		return (ID3D11VertexShader*)m_Impl.m_Current.m_Shader;
	}

	void VertexShaderStateCache::applyToContext(ID3D11DeviceContext* d3dContext)
	{
		if(!m_Impl.m_Dirty)
			return;

		d3dContext->VSSetConstantBuffers(0, Impl::MAX_CONSTBUFFER, m_Impl.m_Current.m_ConstBuffers);
		d3dContext->VSSetSamplers(0, Impl::MAX_SAMPLER, m_Impl.m_Current.m_Samplers);
		d3dContext->VSSetShaderResources(0, Impl::MAX_SRVIEW, m_Impl.m_Current.m_SRViews);
		d3dContext->VSSetShader((ID3D11VertexShader*)m_Impl.m_Current.m_Shader, nullptr, 0);

		m_Impl.m_Dirty = false;
	}
	
	void GeometryShaderStateCache::setShader(ID3D11GeometryShader* shader)
	{
		m_Impl.m_Current.m_Shader = shader;
		m_Impl.m_Dirty = true;
	}

	ID3D11GeometryShader* GeometryShaderStateCache::currentShader()
	{
		return (ID3D11GeometryShader*)m_Impl.m_Current.m_Shader;
	}

	void GeometryShaderStateCache::applyToContext(ID3D11DeviceContext* d3dContext)
	{
		if(!m_Impl.m_Dirty)
			return;

		d3dContext->GSSetConstantBuffers(0, Impl::MAX_CONSTBUFFER, m_Impl.m_Current.m_ConstBuffers);
		d3dContext->GSSetSamplers(0, Impl::MAX_SAMPLER, m_Impl.m_Current.m_Samplers);
		d3dContext->GSSetShaderResources(0, Impl::MAX_SRVIEW, m_Impl.m_Current.m_SRViews);
		d3dContext->GSSetShader((ID3D11GeometryShader*)m_Impl.m_Current.m_Shader, nullptr, 0);

		m_Impl.m_Dirty = false;
	}

	void PixelShaderStateCache::setShader(ID3D11PixelShader* shader)
	{
		m_Impl.m_Current.m_Shader = shader;
		m_Impl.m_Dirty = true;
	}

	ID3D11PixelShader* PixelShaderStateCache::currentShader()
	{
		return (ID3D11PixelShader*)m_Impl.m_Current.m_Shader;
	}

	void PixelShaderStateCache::applyToContext(ID3D11DeviceContext* d3dContext)
	{
		if(!m_Impl.m_Dirty)
			return;

		d3dContext->PSSetConstantBuffers(0, Impl::MAX_CONSTBUFFER, m_Impl.m_Current.m_ConstBuffers);
		d3dContext->PSSetSamplers(0, Impl::MAX_SAMPLER, m_Impl.m_Current.m_Samplers);
		d3dContext->PSSetShaderResources(0, Impl::MAX_SRVIEW, m_Impl.m_Current.m_SRViews);
		d3dContext->PSSetShader((ID3D11PixelShader*)m_Impl.m_Current.m_Shader, nullptr, 0);

		m_Impl.m_Dirty = false;
	}

	//--------------------------------------------------------------------------
	class RenderTargetStateCache::Impl
	{
		friend class RenderTargetStateCache;

		enum {MAX_RTVIEW = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT};
#pragma pack(push)
#pragma pack(1)
		struct State
		{
			ID3D11RenderTargetView* m_RTViews[MAX_RTVIEW];
			ID3D11DepthStencilView* m_DSView;

			State() { memset(this, 0, sizeof(State)); }
		};
#pragma pack(pop)

		State m_Current;
		State m_Backup;
		bool m_Dirty;
	};

	RenderTargetStateCache::RenderTargetStateCache()
		: m_Impl(*new Impl)
	{
		m_Impl.m_Dirty = true;
	}

	RenderTargetStateCache::~RenderTargetStateCache()
	{
		delete &m_Impl;
	}

	void RenderTargetStateCache::create(ID3D11Device* d3dDevice)
	{
		m_Impl.m_Current = Impl::State();
		m_Impl.m_Backup = Impl::State();
		m_Impl.m_Dirty = true;
	}

	void RenderTargetStateCache::destroy()
	{
	}
	
	void RenderTargetStateCache::backup()
	{
		m_Impl.m_Backup = m_Impl.m_Current;
	}

	void RenderTargetStateCache::restore()
	{
		m_Impl.m_Current = m_Impl.m_Backup;
		m_Impl.m_Dirty = true;
	}

	void RenderTargetStateCache::preApplyToContext(ID3D11DeviceContext* d3dContext)
	{
		if(!m_Impl.m_Dirty)
			return;
		
		static ID3D11RenderTargetView* rtViews[Impl::MAX_RTVIEW] = {nullptr};
		static ID3D11DepthStencilView* dsView = nullptr;

		d3dContext->OMSetRenderTargets(Impl::MAX_RTVIEW, rtViews, dsView);
	}

	void RenderTargetStateCache::postApplyToContext(ID3D11DeviceContext* d3dContext)
	{
		if(!m_Impl.m_Dirty)
			return;

		d3dContext->OMSetRenderTargets(
			Impl::MAX_RTVIEW,
			m_Impl.m_Current.m_RTViews,
			m_Impl.m_Current.m_DSView
			);

		m_Impl.m_Dirty = false;
	}

	void RenderTargetStateCache::set(size_t rtvCount, ID3D11RenderTargetView* const * rtViews, ID3D11DepthStencilView *dsView)
	{
		for(size_t i = 0; i < rtvCount; ++i)
			m_Impl.m_Current.m_RTViews[i] = rtViews[i];

		m_Impl.m_Current.m_DSView = dsView;

		m_Impl.m_Dirty = true;
	}

	ID3D11RenderTargetView** RenderTargetStateCache::currentRTView()
	{
		return m_Impl.m_Current.m_RTViews;
	}

	ID3D11DepthStencilView* RenderTargetStateCache::currentDSView()
	{
		return m_Impl.m_Current.m_DSView;
	}

	size_t RenderTargetStateCache::maxNumRTViews() const
	{
		return Impl::MAX_RTVIEW;
	}

}	// namespace js