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

GfxServiceD3D11::GfxServiceD3D11 (Allocator& allocator)
    : buffers_ (16u, allocator)
    , colorTargets_ (16u, allocator)
    , depthTargets_ (16u, allocator)
    , shaderSources_ (16u, allocator)
    , rendShaders_ (16u, allocator)
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
    bool dynamic = (flags & GfxBufferFlags::Dynamic) > 0;
    bool immutable = (flags & GfxBufferFlags::Immutable) > 0;

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

    Buffers::Handle handle = buffers_.acquire ();
    buffers_.construct<BuffersCol> (handle);
    buffers_.get<BuffersCol> (handle)->apiBuffer = buffer;

    return handle.as<HGfxBuffer>();
}

HGfxBuffer GfxServiceD3D11::bufferCreateIndex (size_t sizeInBytes, uint32_t flags, const void* initialData)
{
    bool dynamic = (flags & GfxBufferFlags::Dynamic) > 0;
    bool immutable = (flags & GfxBufferFlags::Immutable) > 0;

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

    Buffers::Handle handle = buffers_.acquire ();
    buffers_.construct<BuffersCol> (handle);
    buffers_.get<BuffersCol> (handle)->apiBuffer = buffer;

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

    Buffers::Handle handle = buffers_.acquire ();
    buffers_.construct<BuffersCol> (handle);
    buffers_.get<BuffersCol> (handle)->apiBuffer = buffer;

    return handle.as<HGfxBuffer>();
}

bool GfxServiceD3D11::bufferDestroy (HGfxBuffer handle)
{
    if (!buffers_.isValid (handle))
        return false;

    if (!buffers_.destruct<BuffersCol> (handle))
        return false;

    return buffers_.release (handle);
}

bool GfxServiceD3D11::bufferUpdate (HGfxBuffer handle, const void* data, size_t dataSize, bool dynamic)
{
    if (!buffers_.isValid (handle) || nullptr == data)
        return false;

    ComObj<ID3D11Buffer> buffer = buffers_.get<BuffersCol> (handle)->apiBuffer;

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
    shaderSources_.construct<ShaderSourcesCol> (handle);
    
    GfxShaderSourceD3D11* source = shaderSources_.get<ShaderSourcesCol> (handle);

    source->dataPtr = shaderSources_.getAllocator().malloc (dataSize);
    source->dataSize = dataSize;

    ::memcpy (source->dataPtr, dataPtr, dataSize);
    
    return handle.as<HGfxShaderSource>();
}

bool GfxServiceD3D11::shaderSourceDestroy (HGfxShaderSource source)
{
    if (!shaderSources_.isValid (source))
        return false;

    GfxShaderSourceD3D11* obj = shaderSources_.get<ShaderSourcesCol> (source);
    shaderSources_.getAllocator().free (obj->dataPtr);

    shaderSources_.destruct <ShaderSourcesCol> (source);
    shaderSources_.release (source);

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
         GfxShaderSourceD3D11* src = shaderSources_.get<ShaderSourcesCol> (vertexSrc);
         D3DBlob* blob = new D3DBlob (src->dataPtr, src->dataSize);
         if (vs.set (apiCreateVertexShader (blob, nullptr)).isNull())
             return HGfxRendShader (0u);
     }

     {
         GfxShaderSourceD3D11* src = shaderSources_.get<ShaderSourcesCol> (fragmentSrc);
         D3DBlob* blob = new D3DBlob (src->dataPtr, src->dataSize);
         if (ps.set (apiCreatePixelShader (blob, nullptr)).isNull())
             return HGfxRendShader (0u);
     }

     RendShaders::Handle handle = rendShaders_.acquire();
     rendShaders_.construct<RendShadersCol>(handle);
     GfxRendShaderD3D11* obj = rendShaders_.get<RendShadersCol>(handle);
     obj->apiVS = vs;
     obj->apiFS = ps;

     return handle.as<HGfxRendShader>();
}

bool GfxServiceD3D11::rendShaderApply (HGfxRendShader shader)
{
    return false;
}

bool GfxServiceD3D11::rendShaderDestroy (HGfxRendShader shader)
{
    if (!rendShaders_.isValid (shader))
        return false;

    rendShaders_.destruct <RendShadersCol> (shader);
    rendShaders_.release (shader);

    return false;
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


}   // namespace
