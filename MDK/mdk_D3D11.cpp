#include "mdk_D3D11.h"

#include <BinaryData.h>
#include <d3dcompiler.h>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace mdk
{

//==============================================================================
D3D11Demo::D3D11Demo()
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
            ErrorReporter r(this);
            if (!d3dStartup())
                return;
        }
            
        {
            ErrorReporter r(this);
            if (!demoStartup())
                return;
        }
            
        while (!thread->threadShouldExit())
        {
            ErrorReporter r(this);
            demoUpdate();
            Thread::yield();
        }

        {
            ErrorReporter r(this);
            demoShutdown();
        }

        {
            ErrorReporter r(this);
            d3dShutdown();
        }
    });
}

bool D3D11Demo::d3dStartup()
{
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
        D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
    };

    if (failed (::D3D11CreateDevice (nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, numElementsInArray(featureLevels), D3D11_SDK_VERSION, d3dDevice_, &d3dFeatureLevel_, d3dIMContext_)))
        return false;
    
    Hold<IDXGIDevice2> dxgiDevice;
    if (failed (d3dDevice_.as (dxgiDevice)))
        return false;

    Hold<IDXGIAdapter> dxgiAdapter;
    if (failed (dxgiDevice->GetAdapter(dxgiAdapter)))
        return false;

    Hold<IDXGIFactory2> dxgiFactory;
    if (failed (dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), dxgiFactory)))
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

    if (failed (dxgiFactory->CreateSwapChainForHwnd (d3dDevice_, (HWND)getTopLevelComponent()->getWindowHandle(), &desc, nullptr, nullptr, d3dSwapchain_)))
        return false;

    Hold<ID3D11Texture2D> d3dBackBuf;
    if (failed (d3dSwapchain_->GetBuffer (0, __uuidof (ID3D11Texture2D), d3dBackBuf)))
        return false;

    if (failed (d3dDevice_->CreateRenderTargetView (d3dBackBuf, nullptr, d3dBackBufRTView_)))
        return false;

    return true;
}

void D3D11Demo::d3dShutdown()
{
    d3dDevice_.set (nullptr);
    d3dIMContext_.set (nullptr);
    d3dSwapchain_.set (nullptr);
}

void D3D11Demo::ShaderCompileDesc::reset()
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

D3D11Demo::ShaderCompileResult D3D11Demo::compileShader (const ShaderCompileDesc& desc)
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

ID3DBlob* D3D11Demo::compileShaderFromBinaryData (const char* id, const char* entry, const char* target)
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

ID3D11VertexShader* D3D11Demo::createVertexShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage)
{
    if (nullptr == bytecode)
        return nullptr;

    Hold<ID3D11VertexShader> shader;
    if (failed (d3dDevice_->CreateVertexShader (bytecode->GetBufferPointer(), bytecode->GetBufferSize(), linkage, shader)))
        return nullptr;

    return shader.drop();
}

ID3D11PixelShader* D3D11Demo::createPixelShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage)
{
    if (nullptr == bytecode)
        return nullptr;

    Hold<ID3D11PixelShader> shader;
    if (failed (d3dDevice_->CreatePixelShader (bytecode->GetBufferPointer(), bytecode->GetBufferSize(), linkage, shader)))
        return nullptr;

    return shader.drop();
}

ID3D11GeometryShader* D3D11Demo::createGeometryShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage)
{
    if (nullptr == bytecode)
        return nullptr;

    Hold<ID3D11GeometryShader> shader;
    if (failed (d3dDevice_->CreateGeometryShader (bytecode->GetBufferPointer(), bytecode->GetBufferSize(), linkage, shader)))
        return nullptr;

    return shader.drop();
}

ID3D11InputLayout* D3D11Demo::createInputLayout (const D3D11_INPUT_ELEMENT_DESC* inputElements, const size_t inputElementCount, ID3DBlob* shaderByteCode)
{
	/*
	std::ostringstream stream;

	stream << "struct VS_INPUT {\n";
	for(size_t i=0; i<inputElementCount; ++i)
	{

	}
	stream << "};\n";
	stream << "struct VS_OUTPUT { float4 vPosition : SV_POSITION; };\n";
	stream << "VS_OUTPUT main(VS_INPUT Input) {\n";
	stream << "VS_OUTPUT Output;\n";
	stream << "Output.vPosition = float4(0, 0, 0, 1);\n";
	stream << "return Output;\n";
	stream << "};\n";

	ID3DBlob* byteCode;
	if(!Shaders::compile(stream.str().c_str(), stream.str().length(), "", "main", "vs_5_0", byteCode))
		return false;
		
	return nullptr;
	*/

	if(nullptr == shaderByteCode)
		return nullptr;

	Hold<ID3D11InputLayout> layout;
	if (failed (d3dDevice_->CreateInputLayout(
		inputElements, inputElementCount,
		shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(),
		layout)))
        return nullptr;

	return layout.drop();
}

ID3D11Buffer* D3D11Demo::createVertexBuffer (size_t sizeInBytes, size_t stride, bool dynamic, const void* initialData)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeInBytes;	// size
	desc.StructureByteStride = stride;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// vertex buffer
	desc.MiscFlags = 0;	// no misc flags
		
	if (dynamic)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;				// gpu read/write
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// cpu write only
	}
	else
	{
		desc.Usage = D3D11_USAGE_DEFAULT;	// gpu read/write
		desc.CPUAccessFlags = 0;			// no cpu sccess
	}

	D3D11_SUBRESOURCE_DATA srdara;
	srdara.pSysMem = initialData;
		
	Hold<ID3D11Buffer> buffer;
	
	if (failed (d3dDevice_->CreateBuffer (&desc, initialData == nullptr ? nullptr : &srdara, buffer)))
        return nullptr;

	return buffer.drop();
}


ID3D11Buffer* D3D11Demo::createIndexBuffer (size_t sizeInBytes, size_t stride, bool dynamic, const void* initialData)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeInBytes;	// size
	desc.StructureByteStride = stride;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;	// index buffer
	desc.MiscFlags = 0;	// no misc flags
		
	if (dynamic)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;				// gpu read/write
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// cpu write only
	}
	else
	{
		desc.Usage = D3D11_USAGE_DEFAULT;	// gpu read/write
		desc.CPUAccessFlags = 0;			// no cpu sccess
	}

	D3D11_SUBRESOURCE_DATA srdara;
	srdara.pSysMem = initialData;

	Hold<ID3D11Buffer> buffer;

	if (failed (d3dDevice_->CreateBuffer (&desc, initialData == nullptr ? nullptr : &srdara, buffer)))
        return nullptr;

	return buffer.drop();
}

ID3D11Buffer* D3D11Demo::createConstantBuffer (size_t sizeInBytes)
{
	if(sizeInBytes % 16 != 0)
		sizeInBytes += 16 - (sizeInBytes % 16);

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = sizeInBytes;

	Hold<ID3D11Buffer> buffer;
	
	if (failed (d3dDevice_->CreateBuffer (&desc, nullptr, buffer)))
        return nullptr;

	return buffer.drop();
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
	
	if (FAILED (d3dDevice->CreateBuffer(&desc, (nullptr == initialData) ? nullptr : &srdata, buffer)))
        return nullptr;

	return buffer.drop();
}

ID3D11Texture2D* D3D11Resource::createTexture2DRenderBuffer (ID3D11Device* d3dDevice, size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, DXGI_FORMAT rtvFormat)
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
	
	if (FAILED (d3dDevice->CreateTexture2D (&desc, nullptr, texture)))
        return nullptr;

	return texture.drop();
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

ID3D11RenderTargetView* D3D11Resource::createRenderTargetView (ID3D11Device* d3dDevice, ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT rtvFormat)
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
	
	if (FAILED (d3dDevice->CreateRenderTargetView (texture, &desc, rtview)))
        return nullptr;

	return rtview.drop();
}

ID3D11DepthStencilView* D3D11Resource::createDepthStencilView (ID3D11Device* d3dDevice, ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT dsvFormat)
{
	D3D11_TEXTURE2D_DESC texdesc;
	texture->GetDesc(&texdesc);
	
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
	
	if (FAILED (d3dDevice->CreateDepthStencilView (texture, &desc, dsview)))
        return nullptr;

	return dsview.drop();
}

ID3D11ShaderResourceView* D3D11Resource::createShaderResourceView (ID3D11Device* d3dDevice, ID3D11Texture2D* texture, DXGI_FORMAT srvFormat)
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
	
	if (FAILED (d3dDevice->CreateShaderResourceView (texture, &desc, srview)))
        return nullptr;

	return srview.drop();
}

ID3D11ShaderResourceView* D3D11Resource::createShaderResourceView (ID3D11Device* d3dDevice, ID3D11Buffer* buffer)
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
	
	if (FAILED (d3dDevice->CreateShaderResourceView (buffer, &desc, srview)))
        return nullptr;

	return srview.drop();
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
	
	if (FAILED (d3dDevice->CreateUnorderedAccessView(buffer, &desc, uaview)))
        return nullptr;

	return uaview.drop();
}

} // namespace
