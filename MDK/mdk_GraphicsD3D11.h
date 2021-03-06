#ifndef MDK_GRAPHICSD3D11_H_INCLUDED
#define MDK_GRAPHICSD3D11_H_INCLUDED

#include "mdk_Graphics.h"

#include "mdk_SOAManager.h"

#pragma warning(disable:4324)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <dxgi.h>
#include <d3d11_1.h>

namespace mdk
{

/*! A small util for holding COM object-pointers
 */
template<class T>
class ComObj
{
public:
    ComObj (T* ptr = nullptr) : ptr_ (ptr)
    {
    }

    ~ComObj()
    {
        if (ptr_)
            ptr_->Release();
    }

    ComObj (const ComObj& src) : ptr_ (nullptr)
    {
        clone (src);
    }

    ComObj& operator = (const ComObj& src)
    {
        return clone (src);
    }

    ComObj& set (T* ptr, bool addRef = false)
    {
        if (ptr_)
            ptr_->Release();

        ptr_ = ptr;

        if ((ptr_ != nullptr) && addRef)
            ptr_->AddRef();

        return *this;
    }

    T* drop()
    {
        T* ret = ptr_;
        ptr_ = nullptr;
        return ret;
    }

    bool isNull() const
    {
        return nullptr == ptr_;
    }

    bool operator == (const T* raw) const
    {
        return ptr_ == raw;
    }

    bool operator != (const T* raw) const
    {
        return ptr_ != raw;
    }

    T* operator -> () const
    {
        return ptr_;
    }

    operator void** ()
    {
        return (void**)&ptr_;
    }

    operator T** ()
    {
        return &ptr_;
    }

    operator T* ()
    {
        return ptr_;
    }

    operator T* () const
    {
        return ptr_;
    }

    template<class U>
    HRESULT as (ComObj<U>& ret)
    {
        U* u;
        HRESULT hr = ptr_->QueryInterface (__uuidof (U), (void**)&u);

        if (SUCCEEDED (hr))
            ret.set (u);

        return hr;
    }

private:
    T* ptr_;

    m_inline ComObj& clone (const ComObj& src)
    {
        set (src.ptr_, true);
        return *this;
    }
};

class GfxBufferD3D11
{
public:
    ComObj<ID3D11Buffer> apiBuffer;
};

class GfxColorTargetD3D11
{
public:
    ComObj<ID3D11RenderTargetView> apiRTV;
    ComObj<ID3D11ShaderResourceView> apiSRV;
};

class GfxDepthTargetD3D11
{
public:
    ComObj<ID3D11DepthStencilView> apiDSV;
    ComObj<ID3D11ShaderResourceView> apiSRV;
};

class GfxShaderSourceD3D11
{
public:
    void* dataPtr;
    size_t dataSize;
};

class GfxRendShaderD3D11
{
public:
    ComObj<ID3D11VertexShader> apiVS;
    ComObj<ID3D11PixelShader> apiPS;
};

class GfxDepthStencilStateD3D11
{
public:
    ComObj<ID3D11DepthStencilState> apiState;
};

class GfxBlendStateD3D11
{
public:
    ComObj<ID3D11BlendState> apiState;
};

class GfxSamplerStateD3D11
{
public:
    ComObj<ID3D11SamplerState> apiState;
};

class GfxServiceD3D11 : public GfxService
{
public:
    GfxServiceD3D11 (Allocator& allocator);
    ~GfxServiceD3D11();

    bool startup (Frontend& frontEnd) override;
    void shutdown() override;

    bool frameBegin() override;
    void frameEnd() override;

    HGfxColorTarget colorTargetDefault() override;
    void colorTargetClear (HGfxColorTarget target, float r, float g, float b, float a) override;
    HGfxDepthTarget depthTargetDefault() override;
    void depthTargetClear (HGfxDepthTarget target, float depth) override;

    HGfxBuffer bufferCreateVertex (size_t sizeInBytes, uint32_t flags, const void* initialData) override;
    HGfxBuffer bufferCreateIndex (size_t sizeInBytes, uint32_t flags, const void* initialData) override;
    HGfxBuffer bufferCreateConstant (size_t sizeInBytes) override;
    bool bufferDestroy (HGfxBuffer buffer) override;
    bool bufferUpdate (HGfxBuffer buffer, const void* data, size_t dataSize, bool dynamic) override;

    HGfxShaderSource shaderSourceCreate (const void* dataPtr, size_t dataSize) override;
    bool shaderSourceDestroy (HGfxShaderSource source) override;

    HGfxRendShader rendShaderCreate (HGfxShaderSource vertexSrc, HGfxShaderSource fragmentSrc) override;
    bool rendShaderApply (HGfxRendShader shader) override;
    bool rendShaderDestroy (HGfxRendShader shader) override;

    HGfxDepthStencilState depthStencilStateCreate (GfxDepthStencilDesc desc) override;
    bool depthStencilStateApply (HGfxDepthStencilState state, uint32_t stencilRefVal) override;
    bool depthStencilStateDestroy (HGfxDepthStencilState state) override;

    HGfxBlendState blendStateCreate (GfxBlendDesc desc) override;
    bool blendStateApply (HGfxBlendState state, float factorR, float factorG, float factorB, float factorA) override;
    bool blendStateDestroy (HGfxBlendState state) override;

    HGfxSamplerState samplerCreate (GfxSamplerDesc desc) override;
    bool samplerDestroy (HGfxSamplerState state) override;
    uint32_t rendShaderSamplerSlotCount() override;
    bool rendShaderVSSetSampler (HGfxSamplerState state, uint32_t slot) override;
    bool rendShaderPSSetSampler (HGfxSamplerState state, uint32_t slot) override;

private:
    typedef SOAManager<SOAManagerTraitsDefault<GfxColorTargetD3D11>> ColorTargets;
    ColorTargets colorTargets_;

    typedef SOAManager<SOAManagerTraitsDefault<GfxDepthTargetD3D11>> DepthTargets;
    DepthTargets depthTargets_;

    typedef SOAManager<SOAManagerTraitsDefault<GfxBufferD3D11>> Buffers;
    Buffers buffers_;

    typedef SOAManager<SOAManagerTraitsDefault<GfxShaderSourceD3D11>> ShaderSources;
    ShaderSources shaderSources_;

    typedef SOAManager<SOAManagerTraitsDefault<GfxRendShaderD3D11>> RendShaders;
    RendShaders rendShaders_;

    typedef SOAManager<SOAManagerTraitsDefault<GfxDepthStencilStateD3D11>> DepthStencilStates;
    DepthStencilStates depthStencilStates_;

    typedef SOAManager<SOAManagerTraitsDefault<GfxBlendStateD3D11>> BlendStates;
    BlendStates blendStates_;
    
    typedef SOAManager<SOAManagerTraitsDefault<GfxSamplerStateD3D11>> SamplerStates;
    SamplerStates samplerStates_;

    class D3DBlob;
    class Mapping;

    D3D_FEATURE_LEVEL apiFeatureLevel_;
    ComObj<ID3D11Device> apiDevice_;
    ComObj<ID3D11DeviceContext> apiContextIM_;
    ComObj<IDXGISwapChain1> apiSwapChain_;
    ComObj<ID3D11RenderTargetView> apiColorBufRTV_;
    ComObj<ID3D11Texture2D> apiDepthBuf_;
    ComObj<ID3D11DepthStencilView> apiDepthBufDSV_;
    ComObj<ID3D11ShaderResourceView> apiDepthBufSRV_;

#if 0
    ComObj<ID3D11SamplerState> sampWrapLinear;
    ComObj<ID3D11SamplerState> sampWrapPoint;
    ComObj<ID3D11SamplerState> sampClampLinear;
    ComObj<ID3D11SamplerState> sampClampPoint;

    ComObj<ID3D11RasterizerState> rastCullNone;
    ComObj<ID3D11RasterizerState> rastCWCullFront;
    ComObj<ID3D11RasterizerState> rastCWCullBack;
    ComObj<ID3D11RasterizerState> rastCCWCullFront;
    ComObj<ID3D11RasterizerState> rastCCWCullBack;

    ComObj<ID3D11DepthStencilState> depthTestOnWriteOn;
    ComObj<ID3D11DepthStencilState> depthTestOnWriteOff;
    ComObj<ID3D11DepthStencilState> depthTestOffWriteOn;
    ComObj<ID3D11DepthStencilState> depthTestOffWriteOff;
#endif

    ID3D11Texture2D* apiCreateTex2D (size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, const void* initialData, size_t rowPitch, size_t slicePitch);
    ID3D11Texture2D* apiCreateTex2DRT (size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, DXGI_FORMAT rtvFormat);
    ID3D11ShaderResourceView* apiCreateSRV (ID3D11Buffer* buffer);
    ID3D11ShaderResourceView* apiCreateSRV (ID3D11Texture2D* texture, DXGI_FORMAT srvFormat);
    ID3D11DepthStencilView* apiCreateDSV (ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT dsvFormat);
    ID3D11VertexShader* apiCreateVertexShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage);
    ID3D11PixelShader* apiCreatePixelShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage);
    
};



}

#endif // MDK_GRAPHICSD3D11_H_INCLUDED
