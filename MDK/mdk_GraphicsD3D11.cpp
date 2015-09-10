#include "mdk_GraphicsD3D11.h"
#include "mdk_FrontendWindows.h"

namespace mdk
{

class GfxServiceD3D11::D3DBlob : public ID3DBlob
{
public:
    void* data;
    size_t size;
    Atomic<ULONG> refCnt;

    D3DBlob (void* ptr, size_t sz)
        : data (ptr)
        , size (sz)
        , refCnt (1u)
    {
    }

    STDMETHOD_ (LPVOID, GetBufferPointer) (void)
    {
        return data;
    }

    STDMETHOD_ (SIZE_T, GetBufferSize) (void)
    {
        return size;
    }

    STDMETHOD (QueryInterface) (REFIID riid, void** ppvObject)
    {
        if (riid == __uuidof (ID3DBlob))
        {
            *ppvObject = this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    STDMETHOD_ (ULONG, AddRef) (void)
    {
        return ++refCnt;
    }

    STDMETHOD_ (ULONG, Release) (void)
    {
        ULONG ret = --refCnt;

        if (ret == 0)
            delete this;

        return ret;
    }
};

class GfxServiceD3D11::Mapping
{
public:
    static D3D11_COMPARISON_FUNC get (GfxCompareFunc::Value value)
    {
        switch (value)
        {
            case GfxCompareFunc::Never: return D3D11_COMPARISON_NEVER;
            case GfxCompareFunc::Less: return D3D11_COMPARISON_LESS;
            case GfxCompareFunc::Equal: return D3D11_COMPARISON_EQUAL;
            case GfxCompareFunc::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
            case GfxCompareFunc::Greater: return D3D11_COMPARISON_GREATER;
            case GfxCompareFunc::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
            case GfxCompareFunc::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
            case GfxCompareFunc::Always: return D3D11_COMPARISON_ALWAYS;
            default: return D3D11_COMPARISON_NEVER;
        }
    }

    static D3D11_STENCIL_OP get (GfxStencilOperation::Value value)
    {
        switch (value)
        {
            case GfxStencilOperation::Keep: return D3D11_STENCIL_OP_KEEP;
            case GfxStencilOperation::Zero: return D3D11_STENCIL_OP_ZERO;
            case GfxStencilOperation::Replace: return D3D11_STENCIL_OP_REPLACE;
            case GfxStencilOperation::IncClamp: return D3D11_STENCIL_OP_INCR_SAT;
            case GfxStencilOperation::DecClamp: return D3D11_STENCIL_OP_DECR_SAT;
            case GfxStencilOperation::Invert: return D3D11_STENCIL_OP_INVERT;
            case GfxStencilOperation::IncWrap: return D3D11_STENCIL_OP_INCR;
            case GfxStencilOperation::DecWrap: return D3D11_STENCIL_OP_DECR;
            default: return D3D11_STENCIL_OP_KEEP;
        }
    }

    static D3D11_BLEND_OP get (GfxBlendOperation::Value value)
    {
        switch (value)
        {
            case GfxBlendOperation::Add: return D3D11_BLEND_OP_ADD;
            case GfxBlendOperation::Subtract: return D3D11_BLEND_OP_SUBTRACT;
            case GfxBlendOperation::ReverseSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
            case GfxBlendOperation::Min: return D3D11_BLEND_OP_MIN;
            case GfxBlendOperation::Max: return D3D11_BLEND_OP_MAX;
            default: return D3D11_BLEND_OP_ADD;
        }
    }

    static D3D11_BLEND get (GfxBlendFactor::Value value)
    {
        switch (value)
        {
            case GfxBlendFactor::Zero: return D3D11_BLEND_ZERO;
            case GfxBlendFactor::One: return D3D11_BLEND_ONE;
            case GfxBlendFactor::SourceColor: return D3D11_BLEND_SRC_COLOR;
            case GfxBlendFactor::OneMinusSourceColor: return D3D11_BLEND_INV_SRC_COLOR;
            case GfxBlendFactor::SourceAlpha: return D3D11_BLEND_SRC_ALPHA;
            case GfxBlendFactor::OneMinusSourceAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
            case GfxBlendFactor::DestinationColor: return D3D11_BLEND_DEST_COLOR;
            case GfxBlendFactor::OneMinusDestinationColor: return D3D11_BLEND_INV_DEST_COLOR;
            case GfxBlendFactor::DestinationAlpha: return D3D11_BLEND_DEST_ALPHA;
            case GfxBlendFactor::OneMinusDestinationAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
            case GfxBlendFactor::SourceAlphaSaturated: return D3D11_BLEND_SRC_ALPHA_SAT;
            case GfxBlendFactor::BlendColor: return D3D11_BLEND_SRC1_COLOR;
            case GfxBlendFactor::OneMinusBlendColor: return D3D11_BLEND_INV_SRC1_COLOR;
            case GfxBlendFactor::BlendAlpha: return D3D11_BLEND_SRC1_ALPHA;
            case GfxBlendFactor::OneMinusBlendAlpha: return D3D11_BLEND_INV_SRC1_ALPHA;
            default: return D3D11_BLEND_ZERO;
        }
    }

    static D3D11_TEXTURE_ADDRESS_MODE get (GfxSamplerAddressMode::Value value)
    {
        switch (value)
        {
            case GfxSamplerAddressMode::ClampToEdge: return D3D11_TEXTURE_ADDRESS_CLAMP;
            case GfxSamplerAddressMode::Repeat: return D3D11_TEXTURE_ADDRESS_WRAP;
            case GfxSamplerAddressMode::MirrorRepeat: return D3D11_TEXTURE_ADDRESS_MIRROR;
            case GfxSamplerAddressMode::ClampToZero: return D3D11_TEXTURE_ADDRESS_BORDER;
            default: return D3D11_TEXTURE_ADDRESS_CLAMP;
        }
    }

    static D3D11_FILTER get (GfxSamplerFilterMode::Value min, GfxSamplerFilterMode::Value mag, GfxSamplerFilterMode::Value mip)
    {
        enum
        {
            D3D11_FILTER_PPP = D3D11_FILTER_MIN_MAG_MIP_POINT,
            D3D11_FILTER_PPL = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
            D3D11_FILTER_PLP = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
            D3D11_FILTER_PLL = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR,
            D3D11_FILTER_LPP = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
            D3D11_FILTER_LPL = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
            D3D11_FILTER_LLP = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
            D3D11_FILTER_LLL = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        };

        unsigned int ret = D3D11_FILTER_MIN_MAG_MIP_POINT;
        
        switch (min)
        {
        case GfxSamplerFilterMode::Linear:
            switch (mag)
            {
            case GfxSamplerFilterMode::Linear: // L L L/P
                ret = GfxSamplerFilterMode::Linear == mip ? D3D11_FILTER_LLL : D3D11_FILTER_LLP;
                break;
            case GfxSamplerFilterMode::Nearest: // L P L/P
                ret = GfxSamplerFilterMode::Linear == mip ? D3D11_FILTER_LPL : D3D11_FILTER_LPP;
                break;
            }
        case GfxSamplerFilterMode::Nearest:
            switch (mag)
            {
            case GfxSamplerFilterMode::Linear: // P L L/P
                ret = GfxSamplerFilterMode::Linear == mip ? D3D11_FILTER_PLL : D3D11_FILTER_PLP;
                break;
            case GfxSamplerFilterMode::Nearest: // P P L/P
                ret = GfxSamplerFilterMode::Linear == mip ? D3D11_FILTER_PPL : D3D11_FILTER_PPP;
                break;
            }
        }

        return (D3D11_FILTER)ret;
    }
};

GfxServiceD3D11::GfxServiceD3D11 (Allocator& allocator)
    : buffers_ (16u, allocator)
    , colorTargets_ (16u, allocator)
    , depthTargets_ (16u, allocator)
    , shaderSources_ (16u, allocator)
    , rendShaders_ (16u, allocator)
    , depthStencilStates_ (16u, allocator)
    , blendStates_ (16u, allocator)
    , samplerStates_ (16u, allocator)
{

}

GfxServiceD3D11::~GfxServiceD3D11()
{
}

bool GfxServiceD3D11::startup (Frontend& frontEnd)
{
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
    };

    if (FAILED (::D3D11CreateDevice (nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, numElementsInArray (featureLevels), D3D11_SDK_VERSION, apiDevice_, &apiFeatureLevel_, apiContextIM_)))
    {
        if (FAILED (::D3D11CreateDevice (nullptr, D3D_DRIVER_TYPE_WARP, nullptr, flags, featureLevels, numElementsInArray (featureLevels), D3D11_SDK_VERSION, apiDevice_, &apiFeatureLevel_, apiContextIM_)))
            return false;
    }

    ComObj<IDXGIDevice2> dxgiDevice;
    if (FAILED (apiDevice_.as (dxgiDevice)))
        return false;

    ComObj<IDXGIAdapter> dxgiAdapter;
    if (FAILED (dxgiDevice->GetAdapter (dxgiAdapter)))
        return false;

    ComObj<IDXGIFactory2> dxgiFactory;
    if (FAILED (dxgiAdapter->GetParent (__uuidof (IDXGIFactory2), dxgiFactory)))
        return false;

    DXGI_SWAP_CHAIN_DESC1 desc;

    desc.Width = 0;
    desc.Height = 0;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Stereo = false;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH; // DXGI_SCALING_NONE is not supported on Windows7
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.Flags = 0;

    HWND hwnd = ((FrontendWinAPI&)frontEnd).hwnd();

    if (FAILED (dxgiFactory->CreateSwapChainForHwnd (apiDevice_, hwnd, &desc, nullptr, nullptr, apiSwapChain_)))
        return false;

    ComObj<ID3D11Texture2D> coloBuf;
    if (FAILED (apiSwapChain_->GetBuffer (0, __uuidof (ID3D11Texture2D), coloBuf)))
        return false;

    if (FAILED (apiDevice_->CreateRenderTargetView (coloBuf, nullptr, apiColorBufRTV_)))
        return false;

    RECT rect;
    ::GetClientRect (hwnd, &rect);
    if (apiDepthBuf_.set (apiCreateTex2DRT (rect.right - rect.left, rect.bottom - rect.top, 1, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT)).isNull())
        return false;

    if (apiDepthBufDSV_.set (apiCreateDSV (apiDepthBuf_, 0, DXGI_FORMAT_D32_FLOAT)).isNull())
        return false;

    if (apiDepthBufSRV_.set (apiCreateSRV (apiDepthBuf_, DXGI_FORMAT_R32_FLOAT)).isNull())
        return false;

    return true;
}

void GfxServiceD3D11::shutdown()
{
}

bool GfxServiceD3D11::frameBegin()
{
    return true;
}

void GfxServiceD3D11::frameEnd()
{
    apiSwapChain_->Present (0u, 0u);
}

HGfxColorTarget GfxServiceD3D11::colorTargetDefault()
{
    return HGfxColorTarget (0u);
}

void GfxServiceD3D11::colorTargetClear (HGfxColorTarget target, float r, float g, float b, float a)
{
    Vec4f rgba (r, g, b, a);
    if (0u == target)
    {
        apiContextIM_->ClearRenderTargetView (apiColorBufRTV_, rgba);
    }
}

HGfxDepthTarget GfxServiceD3D11::depthTargetDefault()
{
    return HGfxDepthTarget (0u);
}

void GfxServiceD3D11::depthTargetClear (HGfxDepthTarget target, float depth)
{
    if (0u == target)
    {
        apiContextIM_->ClearDepthStencilView (apiDepthBufDSV_, 0, depth, 0);
    }
}

HGfxBuffer GfxServiceD3D11::bufferCreateVertex (size_t sizeInBytes, uint32_t flags, const void* initialData)
{
    bool dynamic = (flags & GfxBufferFlag::Dynamic) > 0;
    bool immutable = (flags & GfxBufferFlag::Immutable) > 0;

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = sizeInBytes;   // size
    desc.StructureByteStride = 0;   // for compute buffer only
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;  // vertex buffer
    desc.MiscFlags = 0; // no misc flags

    if (dynamic)
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;               // gpu read/write
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;   // cpu write only
    }
    else
    {
        desc.Usage = immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT;   // gpu read/write
        desc.CPUAccessFlags = 0;                                                // no cpu sccess
    }

    D3D11_SUBRESOURCE_DATA srdara;
    srdara.pSysMem = initialData;

    ComObj<ID3D11Buffer> buffer;

    if (FAILED (apiDevice_->CreateBuffer (&desc, initialData == nullptr ? nullptr : &srdara, buffer)))
        return HGfxBuffer (0);

    Buffers::Handle handle = buffers_.acquire();
    buffers_.construct<0> (handle);
    SOAReadWrite<Buffers> obj (buffers_, handle);
    obj->apiBuffer = buffer;
    
    return handle.as<HGfxBuffer>();
}

HGfxBuffer GfxServiceD3D11::bufferCreateIndex (size_t sizeInBytes, uint32_t flags, const void* initialData)
{
    bool dynamic = (flags & GfxBufferFlag::Dynamic) > 0;
    bool immutable = (flags & GfxBufferFlag::Immutable) > 0;

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = sizeInBytes;   // size
    desc.StructureByteStride = 0;   // for compute buffer only
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;   // index buffer
    desc.MiscFlags = 0;	// no misc flags

    if (dynamic)
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;               // gpu read/write
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;   // cpu write only
    }
    else
    {
        desc.Usage = immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT;   // gpu read/write
        desc.CPUAccessFlags = 0;                                                // no cpu sccess
    }

    D3D11_SUBRESOURCE_DATA srdara;
    srdara.pSysMem = initialData;

    ComObj<ID3D11Buffer> buffer;

    if (FAILED (apiDevice_->CreateBuffer (&desc, initialData == nullptr ? nullptr : &srdara, buffer)))
        return HGfxBuffer (0);

    Buffers::Handle handle = buffers_.acquire();
    buffers_.construct<0> (handle);
    SOAReadWrite<Buffers> obj (buffers_, handle);
    obj->apiBuffer = buffer;

    return handle.as<HGfxBuffer>();
}

HGfxBuffer GfxServiceD3D11::bufferCreateConstant (size_t sizeInBytes)
{
    if (sizeInBytes % 16 != 0)
        sizeInBytes += 16 - (sizeInBytes % 16);

    D3D11_BUFFER_DESC desc;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.ByteWidth = sizeInBytes;

    ComObj<ID3D11Buffer> buffer;

    if (FAILED (apiDevice_->CreateBuffer (&desc, nullptr, buffer)))
        return HGfxBuffer (0);

    Buffers::Handle handle = buffers_.acquire();
    buffers_.construct<0> (handle);

    SOAReadWrite<Buffers> obj (buffers_, handle);
    obj->apiBuffer = buffer;

    return handle.as<HGfxBuffer>();
}

bool GfxServiceD3D11::bufferDestroy (HGfxBuffer handle)
{
    if (!buffers_.destruct<0> (handle))
        return false;

    return buffers_.release (handle);
}

bool GfxServiceD3D11::bufferUpdate (HGfxBuffer handle, const void* data, size_t dataSize, bool dynamic)
{
    if (!buffers_.isValid (handle) || nullptr == data)
        return false;

    ComObj<ID3D11Buffer> buffer;
    {
        buffer = SOARead<Buffers> (buffers_, handle)->apiBuffer;
    }
    if (dynamic)
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (FAILED (apiContextIM_->Map (buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            return false;

        memcpy (mapped.pData, data, dataSize);

        apiContextIM_->Unmap (buffer, 0);

        return true;
    }
    else
    {
        apiContextIM_->UpdateSubresource (buffer, 0, nullptr, data, dataSize, dataSize);
        return true;
    }

    return false;
}

HGfxShaderSource GfxServiceD3D11::shaderSourceCreate (const void* dataPtr, size_t dataSize)
{
    ShaderSources::Handle handle = shaderSources_.acquire();
    shaderSources_.construct <0>(handle);

    SOAReadWrite<ShaderSources> obj (shaderSources_, handle);
    obj->dataPtr = shaderSources_.getAllocator().malloc (dataSize);
    obj->dataSize = dataSize;

    ::memcpy (obj->dataPtr, dataPtr, dataSize);
    
    return handle.as<HGfxShaderSource>();
}

bool GfxServiceD3D11::shaderSourceDestroy (HGfxShaderSource handle)
{
    /*
    SOAReadWrite<ShaderSources> obj (shaderSources_, handle);

    if (!obj.isValid())
        return false;

    shaderSources_.getAllocator().free (obj->dataPtr);
    */
    
    if (!shaderSources_.destruct<0> (handle))
        return false;

    shaderSources_.release (handle);

    return true;
}

HGfxRendShader GfxServiceD3D11::rendShaderCreate (HGfxShaderSource vertexSrc, HGfxShaderSource fragmentSrc)
{
     if (!shaderSources_.isValid (vertexSrc) ||
         !shaderSources_.isValid (fragmentSrc))
         return HGfxRendShader (0u);

     ComObj<ID3D11VertexShader> vs;
     ComObj<ID3D11PixelShader> ps;

     {
         SOARead<ShaderSources> obj (shaderSources_, vertexSrc);
         D3DBlob* blob = new D3DBlob (obj->dataPtr, obj->dataSize);
         if (vs.set (apiCreateVertexShader (blob, nullptr)).isNull())
             return HGfxRendShader (0u);
     }

     {
         SOARead<ShaderSources> obj (shaderSources_, fragmentSrc);
         D3DBlob* blob = new D3DBlob (obj->dataPtr, obj->dataSize);
         if (ps.set (apiCreatePixelShader (blob, nullptr)).isNull())
             return HGfxRendShader (0u);
     }

     RendShaders::Handle handle = rendShaders_.acquire();
     rendShaders_.construct<0> (handle);

     SOAReadWrite<RendShaders> obj (rendShaders_, handle);
     obj->apiVS = vs;
     obj->apiPS = ps;

     return handle.as<HGfxRendShader>();
}

bool GfxServiceD3D11::rendShaderApply (HGfxRendShader handle)
{
    SOARead<RendShaders> obj (rendShaders_, handle);

    if (!obj.isValid())
        return false;

    apiContextIM_->VSSetShader (obj->apiVS, nullptr, 0);
    apiContextIM_->PSSetShader (obj->apiPS, nullptr, 0);

    return true;
}

bool GfxServiceD3D11::rendShaderDestroy (HGfxRendShader handle)
{
    if (!rendShaders_.destruct<0> (handle))
        return false;

    rendShaders_.release (handle);

    return true;
}

uint32_t GfxServiceD3D11::rendShaderSamplerSlotCount()
{
    return D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
}

bool GfxServiceD3D11::rendShaderVSSetSampler (HGfxSamplerState handle, uint32_t slot)
{
    if (slot >= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT)
        return false;

    SOARead<SamplerStates> obj (samplerStates_, handle);
    if (!obj.isValid())
        return false;

    ID3D11SamplerState* apiState = obj->apiState;
    apiContextIM_->VSSetSamplers (slot, 1, &apiState);

    return true;
}

bool GfxServiceD3D11::rendShaderPSSetSampler (HGfxSamplerState handle, uint32_t slot)
{
    if (slot >= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT)
        return false;

    SOARead<SamplerStates> obj (samplerStates_, handle);
    if (!obj.isValid())
        return false;

    ID3D11SamplerState* apiState = obj->apiState;
    apiContextIM_->PSSetSamplers (slot, 1, &apiState);

    return true;
}

ID3D11Texture2D* GfxServiceD3D11::apiCreateTex2D (size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, const void* initialData, size_t rowPitch, size_t slicePitch)
{
    D3D11_TEXTURE2D_DESC desc;
    zerostruct (desc);
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Format = dataFormat;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = mipLevels;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.SampleDesc.Count = 1;

    ComObj<ID3D11Texture2D> texture;

    D3D11_SUBRESOURCE_DATA srdata;
    srdata.pSysMem = initialData;
    srdata.SysMemPitch = rowPitch;
    srdata.SysMemSlicePitch = slicePitch;

    if (FAILED (apiDevice_->CreateTexture2D (&desc, initialData == nullptr ? nullptr : &srdata, texture)))
        return nullptr;

    return texture.drop();
}

ID3D11Texture2D* GfxServiceD3D11::apiCreateTex2DRT (size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, DXGI_FORMAT rtvFormat)
{
    if (-1 == rtvFormat)
        rtvFormat = dataFormat;

    D3D11_TEXTURE2D_DESC desc;
    zerostruct (desc);
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Format = dataFormat;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = mipLevels;
    desc.SampleDesc.Count = 1;

    switch (rtvFormat)
    {
    case ::DXGI_FORMAT_D16_UNORM:
    case ::DXGI_FORMAT_D24_UNORM_S8_UINT:
    case ::DXGI_FORMAT_D32_FLOAT:
    case ::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        break;

    default:
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        break;
    }

    ComObj<ID3D11Texture2D> texture;

    if (FAILED (apiDevice_->CreateTexture2D (&desc, nullptr, texture)))
        return nullptr;

    return texture.drop();
}

ID3D11ShaderResourceView* GfxServiceD3D11::apiCreateSRV (ID3D11Texture2D* texture, DXGI_FORMAT srvFormat)
{
    D3D11_TEXTURE2D_DESC texdesc;
    texture->GetDesc (&texdesc);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    zerostruct (desc);

    if (-1 == srvFormat)
        desc.Format = texdesc.Format;
    else
        desc.Format = srvFormat;

    if (1 == texdesc.ArraySize)
    {
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = texdesc.MipLevels;
        desc.Texture2D.MostDetailedMip = 0;
    }
    else
    {
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.ArraySize = texdesc.ArraySize;
        desc.Texture2DArray.FirstArraySlice = 0;
        desc.Texture2DArray.MipLevels = texdesc.MipLevels;
        desc.Texture2DArray.MostDetailedMip = 0;
    }

    ComObj<ID3D11ShaderResourceView> srview;

    if (FAILED (apiDevice_->CreateShaderResourceView (texture, &desc, srview)))
        return nullptr;

    return srview.drop();
}

ID3D11ShaderResourceView* GfxServiceD3D11::apiCreateSRV (ID3D11Buffer* buffer)
{
    D3D11_BUFFER_DESC bufdesc;
    buffer->GetDesc (&bufdesc);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    zerostruct (desc);

    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = 0;

    if (bufdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        // This is a Raw Buffer
        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
        desc.BufferEx.NumElements = bufdesc.ByteWidth / 4;
    }
    else if (bufdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        // This is a Structured Buffer
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.BufferEx.NumElements = bufdesc.ByteWidth / bufdesc.StructureByteStride;
    }
    else
    {
        return nullptr;
    }

    ComObj<ID3D11ShaderResourceView> srview;

    if (FAILED (apiDevice_->CreateShaderResourceView (buffer, &desc, srview)))
        return nullptr;

    return srview.drop();
}

ID3D11DepthStencilView* GfxServiceD3D11::apiCreateDSV (ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT dsvFormat)
{
    D3D11_TEXTURE2D_DESC texdesc;
    texture->GetDesc (&texdesc);

    D3D11_DEPTH_STENCIL_VIEW_DESC desc;
    zerostruct (desc);

    if (-1 == dsvFormat)
        desc.Format = texdesc.Format;
    else
        desc.Format = dsvFormat;

    if (1 == texdesc.ArraySize)
    {
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = mipLevel;
    }
    else
    {
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.ArraySize = texdesc.ArraySize;
        desc.Texture2DArray.FirstArraySlice = 0;
        desc.Texture2DArray.MipSlice = mipLevel;
    }

    ComObj<ID3D11DepthStencilView> dsview;

    if (FAILED (apiDevice_->CreateDepthStencilView (texture, &desc, dsview)))
        return nullptr;

    return dsview.drop();
}

ID3D11VertexShader* GfxServiceD3D11::apiCreateVertexShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage)
{
    if (nullptr == bytecode)
        return nullptr;

    ComObj<ID3D11VertexShader> shader;
    if (FAILED (apiDevice_->CreateVertexShader (bytecode->GetBufferPointer(), bytecode->GetBufferSize(), linkage, shader)))
        return nullptr;

    return shader.drop();
}

ID3D11PixelShader* GfxServiceD3D11::apiCreatePixelShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage)
{
    if (nullptr == bytecode)
        return nullptr;

    ComObj<ID3D11PixelShader> shader;
    if (FAILED (apiDevice_->CreatePixelShader (bytecode->GetBufferPointer(), bytecode->GetBufferSize(), linkage, shader)))
        return nullptr;

    return shader.drop();
}

HGfxDepthStencilState GfxServiceD3D11::depthStencilStateCreate (GfxDepthStencilDesc desc)
{
    ComObj<ID3D11DepthStencilState> apiObj;

    CD3D11_DEPTH_STENCIL_DESC apiDesc = CD3D11_DEPTH_STENCIL_DESC (CD3D11_DEFAULT());
    apiDesc.DepthEnable = desc.depthCmpFunc != GfxCompareFunc::Always;
    apiDesc.DepthWriteMask = desc.depthWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    apiDesc.DepthFunc = Mapping::get (desc.depthCmpFunc);

    apiDesc.StencilEnable = TRUE;
    apiDesc.StencilReadMask = (UINT8)desc.stencilFrontFace.readMask;
    apiDesc.StencilWriteMask = (UINT8)desc.stencilFrontFace.writeMask;
    
    apiDesc.FrontFace.StencilFunc = Mapping::get (desc.stencilFrontFace.stencilCmpFunc);
    apiDesc.FrontFace.StencilDepthFailOp = Mapping::get (desc.stencilFrontFace.depthFailOp);
    apiDesc.FrontFace.StencilFailOp = Mapping::get (desc.stencilFrontFace.stencilFailOp);
    apiDesc.FrontFace.StencilPassOp = Mapping::get (desc.stencilFrontFace.depthStencilPassOp);

    apiDesc.BackFace.StencilFunc = Mapping::get (desc.stencilBackFace.stencilCmpFunc);
    apiDesc.BackFace.StencilDepthFailOp = Mapping::get (desc.stencilBackFace.depthFailOp);
    apiDesc.BackFace.StencilFailOp = Mapping::get (desc.stencilBackFace.stencilFailOp);
    apiDesc.BackFace.StencilPassOp = Mapping::get (desc.stencilBackFace.depthStencilPassOp);

    if (FAILED (apiDevice_->CreateDepthStencilState (&apiDesc, apiObj)))
        return HGfxDepthStencilState (0u);

    DepthStencilStates::Handle handle = depthStencilStates_.acquire();
    depthStencilStates_.construct<0> (handle);

    SOAReadWrite<DepthStencilStates> obj (depthStencilStates_, handle);
    obj->apiState = apiObj;

    return handle.as<HGfxDepthStencilState>();
}

bool GfxServiceD3D11::depthStencilStateApply (HGfxDepthStencilState handle, uint32_t stencilRefVal)
{
    SOAReadWrite<DepthStencilStates> obj (depthStencilStates_, handle);
    if (!obj.isValid())
        return false;

    apiContextIM_->OMSetDepthStencilState (obj->apiState, stencilRefVal);
    return true;
}

bool GfxServiceD3D11::depthStencilStateDestroy (HGfxDepthStencilState handle)
{
    if (!depthStencilStates_.destruct<0> (handle))
        return false;

    depthStencilStates_.release (handle);

    return true;
}

HGfxBlendState GfxServiceD3D11::blendStateCreate (GfxBlendDesc desc)
{
    ComObj<ID3D11BlendState> apiObj;

    CD3D11_BLEND_DESC apiDesc = CD3D11_BLEND_DESC (CD3D11_DEFAULT());
    apiDesc.AlphaToCoverageEnable = FALSE;
    apiDesc.IndependentBlendEnable= FALSE;
    
    D3D11_RENDER_TARGET_BLEND_DESC& rt0 = apiDesc.RenderTarget[0];
    rt0.BlendEnable = desc.blendEnabled;
    rt0.SrcBlend = Mapping::get (desc.rgbSrcFactor);
    rt0.DestBlend = Mapping::get (desc.rgbDstFactor);
    rt0.BlendOp = Mapping::get (desc.rgbOp);
    rt0.SrcBlendAlpha = Mapping::get (desc.alphaSrcFactor);
    rt0.DestBlendAlpha = Mapping::get (desc.alphaDstFactor);
    rt0.BlendOpAlpha = Mapping::get (desc.alphaOp);
    rt0.RenderTargetWriteMask = 0;
    if (desc.colorWriteMask & GfxColorWriteMask::Red)
        rt0.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED;
    if (desc.colorWriteMask & GfxColorWriteMask::Green)
        rt0.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
    if (desc.colorWriteMask & GfxColorWriteMask::Blue)
        rt0.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
    if (desc.colorWriteMask & GfxColorWriteMask::Alpha)
        rt0.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

    if (FAILED (apiDevice_->CreateBlendState (&apiDesc, apiObj)))
        return HGfxBlendState (0u);

    BlendStates::Handle handle = blendStates_.acquire();
    blendStates_.construct<0> (handle);

    SOAReadWrite<BlendStates> obj (blendStates_, handle);
    obj->apiState = apiObj;

    return handle.as<HGfxBlendState>();
}

bool GfxServiceD3D11::blendStateApply (HGfxBlendState handle, float factorR, float factorG, float factorB, float factorA)
{
    SOARead<BlendStates> obj (blendStates_, handle);
    if (!obj.isValid())
        return false;

    float factor[] = {factorR, factorG, factorB, factorA};
    apiContextIM_->OMSetBlendState (obj->apiState, factor, 0);
    return true;
}

bool GfxServiceD3D11::blendStateDestroy (HGfxBlendState handle)
{
    if (!blendStates_.destruct<0> (handle))
        return false;

    blendStates_.release (handle);
    return true;
}

HGfxSamplerState GfxServiceD3D11::samplerCreate (GfxSamplerDesc desc)
{
    ComObj<ID3D11SamplerState> apiObj;
    CD3D11_SAMPLER_DESC apiDesc = CD3D11_SAMPLER_DESC (CD3D11_DEFAULT());

    apiDesc.Filter = Mapping::get (desc.filterModeMin, desc.filterModeMag, desc.filterModeMip);
    apiDesc.AddressU = Mapping::get (desc.addressModeS);
    apiDesc.AddressV = Mapping::get (desc.addressModeT);
    apiDesc.AddressW = Mapping::get (desc.addressModeR);
    (void)apiDesc.MipLODBias;
    apiDesc.MaxAnisotropy = desc.maxAnisotropy;
    (void)apiDesc.ComparisonFunc;
    (void)apiDesc.BorderColor;
    apiDesc.MinLOD = desc.mipMinLevel;
    apiDesc.MaxLOD = desc.mipMaxLevel;

    if (FAILED (apiDevice_->CreateSamplerState (&apiDesc, apiObj)))
        return HGfxSamplerState (0u);

    SamplerStates::Handle handle = samplerStates_.acquire ();
    samplerStates_.construct<0> (handle);

    SOAReadWrite<SamplerStates> obj (samplerStates_, handle);
    obj->apiState = apiObj;

    return handle.as<HGfxSamplerState>();
}

bool GfxServiceD3D11::samplerDestroy (HGfxSamplerState handle)
{
    if (!samplerStates_.destruct<0> (handle))
        return false;

    samplerStates_.release (handle);
    return true;
}

}   // namespace
