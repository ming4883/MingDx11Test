#include "mdk_D3D11.h"

//#include <d3dcompiler.h>

#pragma comment(lib, "D3D11.lib")
//#pragma comment(lib, "D3DCompiler.lib")

namespace mdk
{

//==============================================================================
D3D11Context::D3D11Context (Demo* demo)
    : demo_ (demo)
{
}

D3D11Context::~D3D11Context()
{
}

bool D3D11Context::startup (void* hwnd)
{
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
    };

    if (m_failed (::D3D11CreateDevice (nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, numElementsInArray (featureLevels), D3D11_SDK_VERSION, device, &featureLevel, contextIM)))
    {
        if (m_failed (::D3D11CreateDevice (nullptr, D3D_DRIVER_TYPE_WARP, nullptr, flags, featureLevels, numElementsInArray (featureLevels), D3D11_SDK_VERSION, device, &featureLevel, contextIM)))
            return false;
    }

    Hold<IDXGIDevice2> dxgiDevice;
    if (m_failed (device.as (dxgiDevice)))
        return false;

    Hold<IDXGIAdapter> dxgiAdapter;
    if (m_failed (dxgiDevice->GetAdapter (dxgiAdapter)))
        return false;

    Hold<IDXGIFactory2> dxgiFactory;
    if (m_failed (dxgiAdapter->GetParent (__uuidof (IDXGIFactory2), dxgiFactory)))
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

    if (m_failed (dxgiFactory->CreateSwapChainForHwnd (device, (HWND)hwnd, &desc, nullptr, nullptr, swapchain)))
        return false;

    Hold<ID3D11Texture2D> backBuf;
    if (m_failed (swapchain->GetBuffer (0, __uuidof (ID3D11Texture2D), backBuf)))
        return false;

    if (m_failed (device->CreateRenderTargetView (backBuf, nullptr, backBufRTView)))
        return false;

    RECT rect;
    ::GetClientRect ((HWND)hwnd, &rect);
    if (m_isnull (depthBuf.set (createTexture2DRT (rect.right - rect.left, rect.bottom - rect.top, 1, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT))))
        return false;

    if (m_isnull (depthBufDSView.set (createDepthStencilView (depthBuf, 0, DXGI_FORMAT_D32_FLOAT))))
        return false;

    if (m_isnull (depthBufSRView.set (createShaderResourceView (depthBuf, DXGI_FORMAT_R32_FLOAT))))
        return false;

    // common sampler states
    {
        D3D11_SAMPLER_DESC _ = CD3D11_SAMPLER_DESC (D3D11_DEFAULT);
        _.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        _.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        _.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        _.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        if (m_isnull (sampWrapLinear.set (createSamplerState (_))))
            return false;
    }
    {
        D3D11_SAMPLER_DESC _ = CD3D11_SAMPLER_DESC (D3D11_DEFAULT);
        _.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        _.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        _.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        _.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        if (m_isnull (sampWrapPoint.set (createSamplerState (_))))
            return false;
    }
    {
        D3D11_SAMPLER_DESC _ = CD3D11_SAMPLER_DESC (D3D11_DEFAULT);
        _.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        _.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        _.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        _.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        if (m_isnull (sampClampLinear.set (createSamplerState (_))))
            return false;
    }
    {
        D3D11_SAMPLER_DESC _ = CD3D11_SAMPLER_DESC (D3D11_DEFAULT);
        _.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        _.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        _.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        _.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        if (m_isnull (sampClampPoint.set (createSamplerState (_))))
            return false;
    }

    // common rasterizer state
    {
        D3D11_RASTERIZER_DESC _ = CD3D11_RASTERIZER_DESC (D3D11_DEFAULT);
        _.CullMode = D3D11_CULL_NONE;
        _.FrontCounterClockwise = FALSE;
        if (m_isnull (rastCullNone.set (createRasterizerState (_))))
            return false;
    }

    {
        D3D11_RASTERIZER_DESC _ = CD3D11_RASTERIZER_DESC (D3D11_DEFAULT);
        _.CullMode = D3D11_CULL_FRONT;
        _.FrontCounterClockwise = FALSE;
        if (m_isnull (rastCWCullFront.set (createRasterizerState (_))))
            return false;
    }

    {
        D3D11_RASTERIZER_DESC _ = CD3D11_RASTERIZER_DESC (D3D11_DEFAULT);
        _.CullMode = D3D11_CULL_BACK;
        _.FrontCounterClockwise = FALSE;
        if (m_isnull (rastCWCullBack.set (createRasterizerState (_))))
            return false;
    }

    {
        D3D11_RASTERIZER_DESC _ = CD3D11_RASTERIZER_DESC (D3D11_DEFAULT);
        _.CullMode = D3D11_CULL_FRONT;
        _.FrontCounterClockwise = TRUE;
        if (m_isnull (rastCCWCullFront.set (createRasterizerState (_))))
            return false;
    }

    {
        D3D11_RASTERIZER_DESC _ = CD3D11_RASTERIZER_DESC (D3D11_DEFAULT);
        _.CullMode = D3D11_CULL_BACK;
        _.FrontCounterClockwise = TRUE;
        if (m_isnull (rastCCWCullBack.set (createRasterizerState (_))))
            return false;
    }

    // common depth stencil state
    {
        D3D11_DEPTH_STENCIL_DESC _ = CD3D11_DEPTH_STENCIL_DESC (D3D11_DEFAULT);
        _.DepthEnable = TRUE;
        _.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        if (m_isnull (depthTestOnWriteOn.set (createDepthStencilState (_))))
            return false;
    }

    {
        D3D11_DEPTH_STENCIL_DESC _ = CD3D11_DEPTH_STENCIL_DESC (D3D11_DEFAULT);
        _.DepthEnable = TRUE;
        _.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        if (m_isnull (depthTestOnWriteOff.set (createDepthStencilState (_))))
            return false;
    }

    {
        D3D11_DEPTH_STENCIL_DESC _ = CD3D11_DEPTH_STENCIL_DESC (D3D11_DEFAULT);
        _.DepthEnable = FALSE;
        _.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        if (m_isnull (depthTestOffWriteOn.set (createDepthStencilState (_))))
            return false;
    }

    {
        D3D11_DEPTH_STENCIL_DESC _ = CD3D11_DEPTH_STENCIL_DESC (D3D11_DEFAULT);
        _.DepthEnable = FALSE;
        _.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        if (m_isnull (depthTestOffWriteOff.set (createDepthStencilState (_))))
            return false;
    }

    return true;
}

void D3D11Context::shutdown()
{
    device.set (nullptr);
    contextIM.set (nullptr);
    swapchain.set (nullptr);
    backBufRTView.set (nullptr);

    depthBuf.set (nullptr);
    depthBufDSView.set (nullptr);
    depthBufSRView.set (nullptr);

    sampWrapLinear.set (nullptr);
    sampWrapPoint.set (nullptr);
    sampClampLinear.set (nullptr);
    sampClampPoint.set (nullptr);

    rastCullNone.set (nullptr);
    rastCWCullFront.set (nullptr);
    rastCWCullBack.set (nullptr);
    rastCCWCullFront.set (nullptr);
    rastCCWCullBack.set (nullptr);

    depthTestOnWriteOn.set (nullptr);
    depthTestOnWriteOff.set (nullptr);
    depthTestOffWriteOn.set (nullptr);
    depthTestOffWriteOff.set (nullptr);
}

#if 0

struct ShaderCompileDesc
{
    const char* source; //!< the HLSL source code
    const char* name;   //!< a name for error messages
    const char* entry;  //!< entry function
    const char* target; //!< e.g. "fx_4_1", "vs_3_0"
    const char* secondaryData;

    D3D_SHADER_MACRO* defines;
    ID3DInclude* include;

    size_t sourceSize;  //!< if 0, set to strlen (source)
    size_t flags1;
    size_t flags2;      //!< ignored for non effect file
    size_t secondaryDataSize;
    size_t secondaryDataFlags;

    ShaderCompileDesc()
    {
        reset();
    }

    void reset();
};

struct ShaderCompileResult
{
    ID3DBlob* byteCode;
    ID3DBlob* errorMessage;

    ShaderCompileResult (ID3DBlob* code, ID3DBlob* error)
        : byteCode (code)
        , errorMessage (error)
    {
    }
};
void ShaderCompileDesc::reset()
{
    source = nullptr;
    name = nullptr;
    entry = nullptr;
    target = nullptr;
    secondaryData = nullptr;

    defines = nullptr;
    include = nullptr;

    sourceSize = 0;
    flags1 = 0;
    flags2 = 0;
    secondaryDataSize = 0;
    secondaryDataFlags = 0;
}

D3D11Context::ShaderCompileResult D3D11Context::compileShader (const ShaderCompileDesc& desc)
{
    size_t sourceSize = desc.sourceSize;
    if (0 == sourceSize)
        sourceSize = strlen (desc.source);

    Hold<ID3DBlob> bytecode;
    Hold<ID3DBlob> error;

    if (FAILED (D3DCompile2 (desc.source, sourceSize, desc.name, desc.defines, desc.include, desc.entry, desc.target, desc.flags1, desc.flags2, desc.secondaryDataFlags, desc.secondaryData, desc.secondaryDataSize, bytecode, error)))
    {
        return ShaderCompileResult (nullptr, error.drop());
    }

    return ShaderCompileResult (bytecode.drop(), error.drop());
}

ID3DBlob* D3D11Context::compileShaderFromBinaryData (const char* id, const char* entry, const char* target)
{
    String fileId = String (id).replaceCharacter ('.', '_');
    int dataSize;
    const char* data = BinaryData::getNamedResource (fileId.toRawUTF8(), dataSize);

    if (nullptr == data)
        return nullptr;

    ShaderCompileDesc desc;
    desc.name = id;
    desc.entry = entry;
    desc.target = target;
    desc.source = data;
    desc.sourceSize = (size_t)dataSize;

    ShaderCompileResult ret = compileShader (desc);

    if (ret.errorMessage)
    {
        const char* str = (const char*)ret.errorMessage->GetBufferPointer();
        errLog (str);
        ret.errorMessage->Release();
    }

    return ret.byteCode;
}
#endif

class MyD3DBlob : public ID3DBlob
{
public:
    HeapBlock<uint8> data;
    size_t size;
    Atomic<ULONG> refCnt;

    MyD3DBlob (size_t sz)
        : data (sz)
        , size (sz)
        , refCnt (1u)
    {
    }

    void copyFrom (const void* src)
    {
        std::memcpy (data.getData(), src, size);
    }

    STDMETHOD_ (LPVOID, GetBufferPointer) (void)
    {
        return data.getData();
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

ID3DBlob* D3D11Context::loadShaderFromAppData (const char* id)
{
    int dataSize;
    const char* data = demo_->appDataGet (id, dataSize);

    if (nullptr == data)
        return nullptr;

    MyD3DBlob* blob = new MyD3DBlob ((size_t)dataSize);
    blob->copyFrom (data);
    return blob;
}

ID3D11VertexShader* D3D11Context::createVertexShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage)
{
    if (nullptr == bytecode)
        return nullptr;

    Hold<ID3D11VertexShader> shader;
    if (m_failed (device->CreateVertexShader (bytecode->GetBufferPointer(), bytecode->GetBufferSize(), linkage, shader)))
        return nullptr;

    return shader.drop();
}

ID3D11PixelShader* D3D11Context::createPixelShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage)
{
    if (nullptr == bytecode)
        return nullptr;

    Hold<ID3D11PixelShader> shader;
    if (m_failed (device->CreatePixelShader (bytecode->GetBufferPointer(), bytecode->GetBufferSize(), linkage, shader)))
        return nullptr;

    return shader.drop();
}

ID3D11GeometryShader* D3D11Context::createGeometryShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage)
{
    if (nullptr == bytecode)
        return nullptr;

    Hold<ID3D11GeometryShader> shader;
    if (m_failed (device->CreateGeometryShader (bytecode->GetBufferPointer(), bytecode->GetBufferSize(), linkage, shader)))
        return nullptr;

    return shader.drop();
}

ID3D11InputLayout* D3D11Context::createInputLayout (const D3D11_INPUT_ELEMENT_DESC* inputElements, const size_t inputElementCount, ID3DBlob* shaderByteCode)
{
    if (nullptr == shaderByteCode)
        return nullptr;

    Hold<ID3D11InputLayout> layout;
    if (m_failed (device->CreateInputLayout (
                      inputElements, inputElementCount,
                      shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(),
                      layout)))
        return nullptr;

    return layout.drop();
}

ID3D11Buffer* D3D11Context::createVertexBuffer (size_t sizeInBytes, bool dynamic, bool immutable, const void* initialData)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = sizeInBytes;	// size
    desc.StructureByteStride = 0;   // for compute buffer only
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// vertex buffer
    desc.MiscFlags = 0;	// no misc flags

    if (dynamic)
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;				// gpu read/write
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// cpu write only
    }
    else
    {
        desc.Usage = immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT;	// gpu read/write
        desc.CPUAccessFlags = 0;			                                    // no cpu sccess
    }

    D3D11_SUBRESOURCE_DATA srdara;
    srdara.pSysMem = initialData;

    Hold<ID3D11Buffer> buffer;

    if (m_failed (device->CreateBuffer (&desc, initialData == nullptr ? nullptr : &srdara, buffer)))
        return nullptr;

    return buffer.drop();
}


ID3D11Buffer* D3D11Context::createIndexBuffer (size_t sizeInBytes, bool dynamic, bool immutable, const void* initialData)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = sizeInBytes;	// size
    desc.StructureByteStride = 0;   // for compute buffer only
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;	// index buffer
    desc.MiscFlags = 0;	// no misc flags

    if (dynamic)
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;				// gpu read/write
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// cpu write only
    }
    else
    {
        desc.Usage = immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT;	// gpu read/write
        desc.CPUAccessFlags = 0;                                                // no cpu sccess
    }

    D3D11_SUBRESOURCE_DATA srdara;
    srdara.pSysMem = initialData;

    Hold<ID3D11Buffer> buffer;

    if (m_failed (device->CreateBuffer (&desc, initialData == nullptr ? nullptr : &srdara, buffer)))
        return nullptr;

    return buffer.drop();
}

ID3D11Buffer* D3D11Context::createConstantBuffer (size_t sizeInBytes)
{
    if (sizeInBytes % 16 != 0)
        sizeInBytes += 16 - (sizeInBytes % 16);

    D3D11_BUFFER_DESC desc;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.ByteWidth = sizeInBytes;

    Hold<ID3D11Buffer> buffer;

    if (m_failed (device->CreateBuffer (&desc, nullptr, buffer)))
        return nullptr;

    return buffer.drop();
}

bool D3D11Context::updateBuffer (ID3D11Buffer* buffer, const void* data, size_t dataSize, bool dynamic)
{
    if (nullptr == buffer || nullptr == data)
        return false;

    if (dynamic)
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (m_failed (contextIM->Map (buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            return false;

        memcpy (mapped.pData, data, dataSize);

        contextIM->Unmap (buffer, 0);

        return true;
    }
    else
    {
        contextIM->UpdateSubresource (buffer, 0, nullptr, data, dataSize, dataSize);
        return true;
    }
}

bool D3D11Context::updateBuffer (ID3D11DeviceContext* context, ID3D11Buffer* buffer, const void* data, size_t dataSize, bool dynamic)
{
    if (nullptr == context || nullptr == buffer || nullptr == data)
        return false;

    if (dynamic)
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (FAILED (context->Map (buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            return false;

        memcpy (mapped.pData, data, dataSize);

        context->Unmap (buffer, 0);

        return true;
    }
    else
    {
        context->UpdateSubresource (buffer, 0, nullptr, data, dataSize, dataSize);
        return true;
    }
}

ID3D11Texture2D* D3D11Context::createTexture2D (size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, const void* initialData, size_t rowPitch, size_t slicePitch)
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

    Hold<ID3D11Texture2D> texture;

    D3D11_SUBRESOURCE_DATA srdata;
    srdata.pSysMem = initialData;
    srdata.SysMemPitch = rowPitch;
    srdata.SysMemSlicePitch = slicePitch;

    if (m_failed (device->CreateTexture2D (&desc, initialData == nullptr ? nullptr : &srdata, texture)))
        return nullptr;

    return texture.drop();
}

ID3D11Texture2D* D3D11Context::createTexture2DFromAppData (const char* id)
{
    int dataSize;
    const char* data = demo_->appDataGet (id, dataSize);

    if (nullptr == data)
        return nullptr;

    juce::Image img = ImageCache::getFromMemory (data, dataSize);

    if (!img.isValid())
        return nullptr;

    int w = img.getWidth();
    int h = img.getHeight();

    if ((w % 2) > 0 || (h % 2) > 0)
    {
        w = (w % 2) > 0 ? (w + 1) : w;
        h = (h % 2) > 0 ? (h + 1) : h;
        img = img.rescaled (w, h);
    }

    img = img.convertedToFormat (juce::Image::ARGB);

    HeapBlock<uint8> mem (w * h * 4);
    memset (mem.getData(), 0xff, w * h * 4);

    juce::Image::BitmapData imgData (img, juce::Image::BitmapData::readOnly);

    uint8* src = imgData.data;
    uint8* dst = mem.getData();
    size_t rowPitch = (size_t)w * 4;

    for (int r = 0; r < h; ++r)
    {
        uint8* s = src;
        uint8* d = dst;

        for (int c = 0; c < w; ++c)
        {
            d[0] = s[2];
            d[1] = s[1];
            d[2] = s[0];
            d[3] = s[3];

            s += 4;
            d += 4;
        }

        src += imgData.lineStride;
        dst += rowPitch;
    }

    Hold<ID3D11Texture2D> texture;
    if (texture.set (createTexture2D (w, h, 1, DXGI_FORMAT_R8G8B8A8_UNORM, mem.getData(), rowPitch, h * rowPitch)).isNull())
        return nullptr;

    return texture.drop();
}

ID3D11Texture2D* D3D11Context::createTexture2DRT (size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, DXGI_FORMAT rtvFormat)
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

    Hold<ID3D11Texture2D> texture;

    if (m_failed (device->CreateTexture2D (&desc, nullptr, texture)))
        return nullptr;

    return texture.drop();
}

ID3D11RenderTargetView* D3D11Context::createRenderTargetView (ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT rtvFormat)
{
    D3D11_TEXTURE2D_DESC texdesc;
    texture->GetDesc (&texdesc);

    D3D11_RENDER_TARGET_VIEW_DESC desc;
    zerostruct (desc);
    if (-1 == rtvFormat)
        desc.Format = texdesc.Format;
    else
        desc.Format = rtvFormat;

    if (1 == texdesc.ArraySize)
    {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = mipLevel;
    }
    else
    {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.ArraySize = texdesc.ArraySize;
        desc.Texture2DArray.FirstArraySlice = 0;
        desc.Texture2DArray.MipSlice = mipLevel;
    }

    Hold<ID3D11RenderTargetView> rtview;

    if (m_failed (device->CreateRenderTargetView (texture, &desc, rtview)))
        return nullptr;

    return rtview.drop();
}

ID3D11DepthStencilView* D3D11Context::createDepthStencilView (ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT dsvFormat)
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

    Hold<ID3D11DepthStencilView> dsview;

    if (m_failed (device->CreateDepthStencilView (texture, &desc, dsview)))
        return nullptr;

    return dsview.drop();
}

ID3D11ShaderResourceView* D3D11Context::createShaderResourceView (ID3D11Texture2D* texture, DXGI_FORMAT srvFormat)
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

    Hold<ID3D11ShaderResourceView> srview;

    if (m_failed (device->CreateShaderResourceView (texture, &desc, srview)))
        return nullptr;

    return srview.drop();
}

ID3D11ShaderResourceView* D3D11Context::createShaderResourceView (ID3D11Buffer* buffer)
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

    Hold<ID3D11ShaderResourceView> srview;

    if (m_failed (device->CreateShaderResourceView (buffer, &desc, srview)))
        return nullptr;

    return srview.drop();
}

ID3D11SamplerState* D3D11Context::createSamplerState (const D3D11_SAMPLER_DESC& desc)
{
    Hold<ID3D11SamplerState> state;
    if (m_failed (device->CreateSamplerState (&desc, state)))
        return nullptr;

    return state.drop();
}

ID3D11RasterizerState* D3D11Context::createRasterizerState (const D3D11_RASTERIZER_DESC& desc)
{
    Hold<ID3D11RasterizerState> state;
    if (m_failed (device->CreateRasterizerState (&desc, state)))
        return nullptr;

    return state.drop();
}

ID3D11DepthStencilState* D3D11Context::createDepthStencilState (const D3D11_DEPTH_STENCIL_DESC& desc)
{
    Hold<ID3D11DepthStencilState> state;
    if (m_failed (device->CreateDepthStencilState (&desc, state)))
        return nullptr;

    return state.drop();
}
//==============================================================================
D3D11Demo::D3D11Demo()
    : d3d11 (this)
{
}

D3D11Demo::~D3D11Demo()
{
}

void D3D11Demo::paint (Graphics&)
{
    if (renderThreadExists())
        return;

    renderThreadStart ([this] (Thread* thread)
    {
        {
            ErrorReporter r (this);
            if (!d3d11.startup (getTopLevelComponent()->getWindowHandle()))
                return;
        }

        {
            ErrorReporter r (this);
            if (!demoStartup())
                return;
        }

        timeInit();

        while (!thread->threadShouldExit())
        {
            ErrorReporter r (this);
            timeUpdate();
            demoUpdate();
            Thread::yield();
        }

        {
            ErrorReporter r (this);
            demoShutdown();
        }

        {
            ErrorReporter r (this);
            d3d11.shutdown();
        }
    });
}


D3D11_VIEWPORT D3D11Demo::getViewport (float x_ratio, float y_ratio, float w_ratio, float h_ratio, float minz, float maxz)
{
    auto bounds = getBounds();
    D3D11_VIEWPORT ret;

    ret.Width = bounds.getWidth() * w_ratio;
    ret.Height = bounds.getHeight() * h_ratio;
    ret.TopLeftX = bounds.getWidth() * x_ratio;
    ret.TopLeftY = bounds.getHeight() * y_ratio;
    ret.MinDepth = minz;
    ret.MaxDepth = maxz;
    return ret;
}

float D3D11Demo::getAspect()
{
    auto bounds = getBounds();
    return bounds.getWidth() / (float)bounds.getHeight();
}



//==============================================================================

ID3D11Buffer* D3D11Resource::createStructComputeBuffer (ID3D11Device* d3dDevice, size_t bufferSizeInBytes, size_t structSizeInBytes, const void* initialData)
{
    //if(bufferSizeInBytes % 16 != 0)
    //	bufferSizeInBytes += 16 - (bufferSizeInBytes % 16);
    D3D11_BUFFER_DESC desc;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.ByteWidth = bufferSizeInBytes;
    desc.StructureByteStride = structSizeInBytes;

    D3D11_SUBRESOURCE_DATA srdata;
    srdata.pSysMem = initialData;

    Hold<ID3D11Buffer> buffer;

    if (FAILED (d3dDevice->CreateBuffer (&desc, (nullptr == initialData) ? nullptr : &srdata, buffer)))
        return nullptr;

    return buffer.drop();
}

ID3D11Texture2D* D3D11Resource::createTexture2DArrayRenderBuffer (ID3D11Device* d3dDevice, size_t width, size_t height, size_t arraySize, size_t mipLevels, DXGI_FORMAT dataFormat, DXGI_FORMAT rtvFormat)
{
    if (-1 == rtvFormat)
        rtvFormat = dataFormat;

    D3D11_TEXTURE2D_DESC desc;
    zerostruct (desc);
    desc.ArraySize = arraySize;
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

    Hold<ID3D11Texture2D> texture;
    if (FAILED (d3dDevice->CreateTexture2D (&desc, nullptr, texture)))
        return nullptr;

    return texture.drop();
}

ID3D11Texture2D* D3D11Resource::createTexture2DStagingBuffer (ID3D11Device* d3dDevice, size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat)
{
    D3D11_TEXTURE2D_DESC desc;
    zerostruct (desc);
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.Format = dataFormat;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = mipLevels;
    desc.SampleDesc.Count = 1;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    Hold<ID3D11Texture2D> texture;

    if (FAILED (d3dDevice->CreateTexture2D (&desc, nullptr, texture)))
        return nullptr;

    return texture.drop();
}

ID3D11UnorderedAccessView* D3D11Resource::createUnorderedAccessView (ID3D11Device* d3dDevice, ID3D11Buffer* buffer)
{
    D3D11_BUFFER_DESC bufdesc;
    buffer->GetDesc (&bufdesc);

    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    zerostruct (desc);

    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;

    if (bufdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        // This is a Raw Buffer
        desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
        desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
        desc.Buffer.NumElements = bufdesc.ByteWidth / 4;
    }
    else if (bufdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        // This is a Structured Buffer
        desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
        desc.Buffer.NumElements = bufdesc.ByteWidth / bufdesc.StructureByteStride;
    }
    else
    {
        return nullptr;
    }

    Hold<ID3D11UnorderedAccessView> uaview;

    if (FAILED (d3dDevice->CreateUnorderedAccessView (buffer, &desc, uaview)))
        return nullptr;

    return uaview.drop();
}

} // namespace
