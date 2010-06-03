#include "DXUT.h"
#include "Buffers.h"

#include <sstream>

namespace js
{
	ID3D11InputLayout* Buffers::createInputLayout(
		ID3D11Device* d3dDevice,
		const D3D11_INPUT_ELEMENT_DESC* inputElements,
		const size_t inputElementCount,
		ID3DBlob* shaderByteCode)
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

		ID3D11InputLayout* layout = nullptr;
		HRESULT hr = d3dDevice->CreateInputLayout(
			inputElements, inputElementCount,
			shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(),
			&layout);

		if(FAILED(hr)) js_safe_release(layout);

		return layout;
	}

	ID3D11Buffer* Buffers::createVertexBuffer(
		ID3D11Device* d3dDevice,
		size_t sizeInBytes,
		size_t stride,
		bool dynamic,
		const void* initialData)
	{
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeInBytes;	// size
		desc.StructureByteStride = stride;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// vertex buffer
		desc.MiscFlags = 0;	// no misc flags
		
		if(dynamic)
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
		
		ID3D11Buffer* buffer = nullptr;
		HRESULT hr = d3dDevice->CreateBuffer(&desc, initialData == nullptr ? nullptr : &srdara, &buffer);

		if(FAILED(hr)) js_safe_release(buffer);

		return buffer;
	}

	ID3D11Buffer* Buffers::createIndexBuffer(
		ID3D11Device* d3dDevice,
		size_t sizeInBytes,
		size_t stride,
		bool dynamic,
		const void* initialData)
	{
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeInBytes;	// size
		desc.StructureByteStride = stride;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;	// index buffer
		desc.MiscFlags = 0;	// no misc flags
		
		if(dynamic)
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

		ID3D11Buffer* buffer = nullptr;
		HRESULT hr = d3dDevice->CreateBuffer(&desc, initialData == nullptr ? nullptr : &srdara, &buffer);

		if(FAILED(hr)) js_safe_release(buffer);

		return buffer;
	}

	ID3D11Buffer* Buffers::createConstantBuffer(
		ID3D11Device* d3dDevice,
		size_t sizeInBytes)
	{
		if(sizeInBytes % 16 != 0)
			sizeInBytes += 16 - (sizeInBytes % 16);

		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.ByteWidth = sizeInBytes;

		ID3D11Buffer* buffer = nullptr;
		HRESULT hr = d3dDevice->CreateBuffer(&desc, nullptr, &buffer);

		if(FAILED(hr)) js_safe_release(buffer);

		return buffer;
	}

	ID3D11Buffer* Buffers::createStructComputeBuffer(
		ID3D11Device* d3dDevice,
		size_t bufferSizeInBytes,
		size_t structSizeInBytes,
		const void* initialData)
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

		ID3D11Buffer* buffer = nullptr;
		HRESULT hr = d3dDevice->CreateBuffer(&desc, (nullptr == initialData) ? nullptr : &srdata, &buffer);
		
		if(FAILED(hr)) js_safe_release(buffer);

		return buffer;
	}

	ID3D11Texture2D* Buffers::createTexture2DRenderBuffer(
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		size_t mipLevels,
		DXGI_FORMAT dataFormat,
		DXGI_FORMAT rtvFormat)
	{
		if(-1 == rtvFormat) rtvFormat = dataFormat;

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.ArraySize = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.Format = dataFormat;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = mipLevels;
		desc.SampleDesc.Count = 1;
		
		switch(rtvFormat)
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

		ID3D11Texture2D* texture = nullptr;
		HRESULT hr = d3dDevice->CreateTexture2D(&desc, nullptr, &texture);

		if(FAILED(hr)) js_safe_release(texture);

		return texture;
	}

	ID3D11Texture2D* Buffers::createTexture2DArrayRenderBuffer(
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		size_t arraySize,
		size_t mipLevels,
		DXGI_FORMAT dataFormat,
		DXGI_FORMAT rtvFormat)
	{
		if(-1 == rtvFormat) rtvFormat = dataFormat;

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.ArraySize = arraySize;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.Format = dataFormat;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = mipLevels;
		desc.SampleDesc.Count = 1;

		switch(rtvFormat)
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

		ID3D11Texture2D* texture = nullptr;
		HRESULT hr = d3dDevice->CreateTexture2D(&desc, nullptr, &texture);

		if(FAILED(hr)) js_safe_release(texture);

		return texture;
	}

	ID3D11Texture2D* Buffers::createTexture2DStagingBuffer(
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		size_t mipLevels,
		DXGI_FORMAT dataFormat)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.ArraySize = 1;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.Format = dataFormat;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = mipLevels;
		desc.SampleDesc.Count = 1;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		ID3D11Texture2D* texture = nullptr;
		HRESULT hr = d3dDevice->CreateTexture2D(&desc, nullptr, &texture);

		if(FAILED(hr)) js_safe_release(texture);

		return texture;
	}

	ID3D11RenderTargetView* Buffers::createRenderTargetView(
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		size_t mipLevel,
		DXGI_FORMAT rtvFormat)
	{
		
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		D3D11_TEXTURE2D_DESC texdesc;
		texture->GetDesc(&texdesc);
		
		if(-1 == rtvFormat)
			desc.Format = texdesc.Format;
		else
			desc.Format = rtvFormat;

		if(1 == texdesc.ArraySize)
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

		ID3D11RenderTargetView* rtview = nullptr;
		HRESULT hr = d3dDevice->CreateRenderTargetView(texture, &desc, &rtview);

		if(FAILED(hr)) js_safe_release(rtview);

		return rtview;
	}

	ID3D11DepthStencilView* Buffers::createDepthStencilView(
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		size_t mipLevel,
		DXGI_FORMAT dsvFormat)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		D3D11_TEXTURE2D_DESC texdesc;
		texture->GetDesc(&texdesc);
		
		if(-1 == dsvFormat)
			desc.Format = texdesc.Format;
		else
			desc.Format = dsvFormat;

		if(1 == texdesc.ArraySize)
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

		ID3D11DepthStencilView* dsview = nullptr;
		HRESULT hr = d3dDevice->CreateDepthStencilView(texture, &desc, &dsview);

		if(FAILED(hr)) js_safe_release(dsview);

		return dsview;
	}

	ID3D11ShaderResourceView* Buffers::createShaderResourceView(
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		DXGI_FORMAT srvFormat)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		D3D11_TEXTURE2D_DESC texdesc;
		texture->GetDesc(&texdesc);
		
		if(-1 == srvFormat)	
			desc.Format = texdesc.Format;
		else
			desc.Format = srvFormat;

		if(1 == texdesc.ArraySize)
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

		ID3D11ShaderResourceView* srview = nullptr;
		HRESULT hr = d3dDevice->CreateShaderResourceView(texture, &desc, &srview);

		if(FAILED(hr)) js_safe_release(srview);

		return srview;
	}

	 ID3D11ShaderResourceView* Buffers::createShaderResourceView(
		ID3D11Device* d3dDevice,
		ID3D11Buffer* buffer)
	 {
		 D3D11_BUFFER_DESC bufdesc;
		 ZeroMemory(&bufdesc, sizeof(bufdesc));
		 buffer->GetDesc(&bufdesc);

		 D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		 ZeroMemory(&desc, sizeof(desc));
		 desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		 desc.BufferEx.FirstElement = 0;

		 if(bufdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
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

		 ID3D11ShaderResourceView* srview = nullptr;
		 HRESULT hr = d3dDevice->CreateShaderResourceView(buffer, &desc, &srview);

		 if(FAILED(hr)) js_safe_release(srview);

		 return srview;
	 }

	 ID3D11UnorderedAccessView* Buffers::createUnorderedAccessView(
		ID3D11Device* d3dDevice,
		ID3D11Buffer* buffer)
	 {
		 D3D11_BUFFER_DESC bufdesc;
		 ZeroMemory(&bufdesc, sizeof(bufdesc));
		 buffer->GetDesc(&bufdesc);

		 D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		 ZeroMemory(&desc, sizeof(desc));
		 desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		 desc.Buffer.FirstElement = 0;

		 if(bufdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
		 {
			 // This is a Raw Buffer
			 desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
			 desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			 desc.Buffer.NumElements = bufdesc.ByteWidth / 4; 
		 }
		 else if(bufdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		 {
			 // This is a Structured Buffer
			 desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
			 desc.Buffer.NumElements = bufdesc.ByteWidth / bufdesc.StructureByteStride; 
		 } 
		 else
		 {
			 return nullptr;
		 }

		 ID3D11UnorderedAccessView* uaview = nullptr;
		 HRESULT hr = d3dDevice->CreateUnorderedAccessView(buffer, &desc, &uaview);

		 if(FAILED(hr)) js_safe_release(uaview);

		 return uaview;
	 }

	//--------------------------------------------------------------------------
	 Texture2DRenderBuffer::Texture2DRenderBuffer()
		: m_TextureObject(nullptr)
	{
	}

	Texture2DRenderBuffer::~Texture2DRenderBuffer()
	{
		destroy();
	}

	bool Texture2DRenderBuffer::valid() const
	{
		return m_TextureObject != nullptr;
	}

	void Texture2DRenderBuffer::create(
		ID3D11Device* d3dDevice,
		size_t width, size_t height, size_t mipLevels,
		DXGI_FORMAT dataFormat,
		DXGI_FORMAT rtvFormat,
		DXGI_FORMAT srvFormat)
	{
		m_TextureObject = Buffers::createTexture2DRenderBuffer(d3dDevice, width, height, mipLevels, dataFormat, rtvFormat);
		if(nullptr == m_TextureObject) return;

		m_Width = width; m_Height = height; m_Format = dataFormat; m_MipLevels = mipLevels;

		if(-1 == rtvFormat) rtvFormat = dataFormat;
		if(-1 == srvFormat) srvFormat = dataFormat;

		switch(rtvFormat)
		{
		case ::DXGI_FORMAT_D16_UNORM:
		case ::DXGI_FORMAT_D24_UNORM_S8_UINT:
		case ::DXGI_FORMAT_D32_FLOAT:
		case ::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			m_DSView = Buffers::createDepthStencilView(d3dDevice, m_TextureObject, 0, rtvFormat);
			break;

		default:
			m_RTView = Buffers::createRenderTargetView(d3dDevice, m_TextureObject, 0, rtvFormat);
			break;
		}

		if(nullptr == m_DSView && nullptr == m_RTView)
		{
			destroy();
			return;
		}

		// shader resource view
		m_SRView = Buffers::createShaderResourceView(d3dDevice, m_TextureObject, srvFormat);

		if(nullptr == m_SRView)
		{
			destroy();
			return;
		}
	}

	void Texture2DRenderBuffer::destroy()
	{
		js_safe_release(m_SRView);
		js_safe_release(m_DSView);
		js_safe_release(m_RTView);
		js_safe_release(m_TextureObject);

		m_Width = 0;
		m_Height = 0;
		m_MipLevels = 0;
		m_Format = (DXGI_FORMAT)-1;
	}

	//--------------------------------------------------------------------------
	Texture2DArrayRenderBuffer::Texture2DArrayRenderBuffer()
		: m_TextureObject(nullptr)
	{
	}

	Texture2DArrayRenderBuffer::~Texture2DArrayRenderBuffer()
	{
		destroy();
	}

	bool Texture2DArrayRenderBuffer::valid() const
	{
		return m_TextureObject != nullptr;
	}

	void Texture2DArrayRenderBuffer::create(
		ID3D11Device* d3dDevice,
		size_t width, size_t height, size_t arraySize, size_t mipLevels,
		DXGI_FORMAT dataFormat,
		DXGI_FORMAT rtvFormat,
		DXGI_FORMAT srvFormat)
	{
		m_TextureObject = Buffers::createTexture2DRenderBuffer(d3dDevice, width, height, mipLevels, dataFormat, rtvFormat);
		if(nullptr == m_TextureObject) return;

		m_Width = width; m_Height = height; m_ArraySize = arraySize; m_Format = dataFormat; m_MipLevels = mipLevels;

		if(-1 == rtvFormat) rtvFormat = dataFormat;
		if(-1 == srvFormat) srvFormat = dataFormat;

		switch(rtvFormat)
		{
		case ::DXGI_FORMAT_D16_UNORM:
		case ::DXGI_FORMAT_D24_UNORM_S8_UINT:
		case ::DXGI_FORMAT_D32_FLOAT:
		case ::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			m_DSView = Buffers::createDepthStencilView(d3dDevice, m_TextureObject, 0, rtvFormat);
			break;

		default:
			m_RTView = Buffers::createRenderTargetView(d3dDevice, m_TextureObject, 0, rtvFormat);
			break;
		}

		if(nullptr == m_DSView && nullptr == m_RTView)
		{
			destroy();
			return;
		}

		// shader resource view
		m_SRView = Buffers::createShaderResourceView(d3dDevice, m_TextureObject, srvFormat);

		if(nullptr == m_SRView)
		{
			destroy();
			return;
		}
	}

	void Texture2DArrayRenderBuffer::destroy()
	{
		js_safe_release(m_SRView);
		js_safe_release(m_DSView);
		js_safe_release(m_RTView);
		js_safe_release(m_TextureObject);

		m_Width = 0;
		m_Height = 0;
		m_ArraySize = 0;
		m_MipLevels = 0;
		m_Format = (DXGI_FORMAT)-1;
	}

}	// namespace js