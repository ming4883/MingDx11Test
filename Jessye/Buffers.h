#ifndef BUFFERS_H
#define BUFFERS_H

#include "Platform.h"

namespace js
{

struct Buffers
{
	static ID3D11InputLayout* createInputLayout(
		ID3D11Device* d3dDevice,
		const D3D11_INPUT_ELEMENT_DESC* inputElements,
		const size_t inputElementCount,
		ID3DBlob* shaderByteCode);

	/*! Create a vertex buffer which allows:
		- gpu read / write
		- non dynamic usage (i.e. dynamic writing may be heavy)
		- cpu write only
	*/
	static ID3D11Buffer* createVertexBuffer(
		ID3D11Device* d3dDevice,
		size_t sizeInBytes,
		size_t stride,
		bool dynamic,
		void* initialData = nullptr);

	/*! Create a index buffer which allows:
		- gpu read / write
		- non dynamic usage (i.e. dynamic writing may be heavy)
		- cpu write only
	*/
	static ID3D11Buffer* createIndexBuffer(
		ID3D11Device* d3dDevice,
		size_t sizeInBytes,
		size_t stride,
		bool dynamic,
		void* initialData = nullptr);

	static ID3D11Buffer* createConstantBuffer(
		ID3D11Device* d3dDevice,
		size_t sizeInBytes);

	static ID3D11Buffer* createStructComputeBuffer(
		ID3D11Device* d3dDevice,
		size_t bufferSizeInBytes,
		size_t structSizeInBytes,
		void* initialData = nullptr);

	static ID3D11Texture2D* createTexture2DRenderBuffer(
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		size_t mipLevels,
		DXGI_FORMAT dataFormat,
		DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);
	
	static ID3D11Texture2D* createTexture2DArrayRenderBuffer(
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		size_t arraySize,
		size_t mipLevels,
		DXGI_FORMAT dataFormat,
		DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);

	static ID3D11Texture2D* createTexture2DStagingBuffer(
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		size_t mipLevels,
		DXGI_FORMAT dataFormat);

	static ID3D11RenderTargetView* createRenderTargetView(
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		size_t mipLevel,
		DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);
	
	static ID3D11DepthStencilView* createDepthStencilView(
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		size_t mipLevel,
		DXGI_FORMAT dsvFormat = (DXGI_FORMAT)-1);

	static ID3D11ShaderResourceView* createShaderResourceView(
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		DXGI_FORMAT srvFormat = (DXGI_FORMAT)-1);

	static ID3D11ShaderResourceView* createShaderResourceView(
		ID3D11Device* d3dDevice,
		ID3D11Buffer* buffer);
	
	static ID3D11UnorderedAccessView* createUnorderedAccessView(
		ID3D11Device* d3dDevice,
		ID3D11Buffer* buffer);
		
};	// Buffers

//! Template for render buffer
template<typename T>
struct ConstantBuffer_t
{
	ID3D11Buffer* m_BufferObject;
	T m_SysMemCopy;
	void* m_MappedPtr;

	ConstantBuffer_t() : m_BufferObject(nullptr), m_MappedPtr(nullptr)
	{
	}

	~ConstantBuffer_t()
	{
		destroy();
	}

	bool valid() const
	{
		return m_BufferObject != nullptr;
	}

	void create(ID3D11Device* d3dDevice)
	{
		m_BufferObject = Buffers::createConstantBuffer(d3dDevice, sizeof(T));
	}

	void destroy()
	{
		js_safe_release(m_BufferObject);
	}

	operator ID3D11Buffer* () { return m_BufferObject; }

	void map(ID3D11DeviceContext* d3dContext)
	{
		if(!valid())
			return;

		js_assert(nullptr == m_MappedPtr);

		D3D11_MAPPED_SUBRESOURCE mapped;
		if(FAILED(d3dContext->Map(m_BufferObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
			return;
		
		m_MappedPtr = mapped.pData;
	}
	
	void unmap(ID3D11DeviceContext* d3dContext)
	{
		if(!valid())
			return;
		
		js_assert(nullptr != m_MappedPtr);

		memcpy(m_MappedPtr, &m_SysMemCopy, sizeof(T));

		d3dContext->Unmap(m_BufferObject, 0);
		m_MappedPtr = nullptr;
	}

	T& data()
	{
		js_assert(nullptr != m_MappedPtr);
		//return *static_cast<T*>(m_MappedPtr);
		return m_SysMemCopy;
	}

};

//! StructComputeBuffer
struct StructComputeBuffer
{
	ID3D11Buffer* m_BufferObject;
	ID3D11ShaderResourceView* m_SRView;
	ID3D11UnorderedAccessView* m_UAView;

	StructComputeBuffer() : m_BufferObject(nullptr), m_SRView(nullptr), m_UAView(nullptr)
	{
	}

	~StructComputeBuffer()
	{
		destroy();
	}

	bool valid() const
	{
		return (m_BufferObject != nullptr)
			&& (m_SRView != nullptr)
			&& (m_UAView != nullptr);
	}

	void create(ID3D11Device* d3dDevice, size_t bufferSizeInBytes, size_t structSizeInBytes, void* initialData = nullptr)
	{
		m_BufferObject = Buffers::createStructComputeBuffer(d3dDevice, bufferSizeInBytes, structSizeInBytes, initialData);
		m_SRView = Buffers::createShaderResourceView(d3dDevice, m_BufferObject);
		m_UAView = Buffers::createUnorderedAccessView(d3dDevice, m_BufferObject);
	}

	void destroy()
	{
		js_safe_release(m_UAView);
		js_safe_release(m_SRView);
		js_safe_release(m_BufferObject);
	}

	operator ID3D11Buffer* () { return m_BufferObject; }
	operator ID3D11ShaderResourceView* () { return m_SRView; }
	operator ID3D11UnorderedAccessView* () { return m_UAView; }
};

//! Texture2DStagingBuffer
struct Texture2DStagingBuffer
{
	ID3D11Texture2D* m_TextureObject;

	size_t m_Width;
	size_t m_Height;
	size_t m_MipLevels;
	DXGI_FORMAT m_Format;
	void* m_MappedPtr;

	Texture2DStagingBuffer()
		: m_TextureObject(nullptr)
		, m_Width(0)
		, m_Height(0)
		, m_MipLevels(0)
		, m_Format((DXGI_FORMAT)-1)
		, m_MappedPtr(nullptr)
	{
	}

	~Texture2DStagingBuffer()
	{
		destroy();
	}

	bool valid() const
	{
		return m_TextureObject != nullptr;
	}

	void create(ID3D11Device* d3dDevice, size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat)
	{
		m_TextureObject = Buffers::createTexture2DStagingBuffer(d3dDevice, width, height, mipLevels, dataFormat);
		if(nullptr == m_TextureObject) return;

		m_Width = width; m_Height = height; m_Format = dataFormat; m_MipLevels = mipLevels;
	}

	void destroy()
	{
		js_safe_release(m_TextureObject);

		m_Width = 0;
		m_Height = 0;
		m_MipLevels = 0;
		m_Format = (DXGI_FORMAT)-1;
	}

	void map(ID3D11DeviceContext* d3dContext, D3D11_MAP mapOptions)
	{
		if(!valid())
			return;

		js_assert(nullptr == m_MappedPtr);

		D3D11_MAPPED_SUBRESOURCE mapped;
		if(FAILED(d3dContext->Map(m_TextureObject, 0, mapOptions, 0, &mapped)))
			return;
		
		m_MappedPtr = mapped.pData;
	}
	
	void unmap(ID3D11DeviceContext* d3dContext)
	{
		if(!valid())
			return;
		
		js_assert(nullptr != m_MappedPtr);

		d3dContext->Unmap(m_TextureObject, 0);
		m_MappedPtr = nullptr;
	}

	template<typename T>
	T* data()
	{
		js_assert(nullptr != m_MappedPtr);
		return static_cast<T*>(m_MappedPtr);
	}


};	// Texture2DStagingBuffer

//! Texture2DRenderBuffer
struct Texture2DRenderBuffer
{
	ID3D11Texture2D* m_TextureObject;
	ID3D11RenderTargetView* m_RTView;
	ID3D11DepthStencilView* m_DSView;
	ID3D11ShaderResourceView* m_SRView;

	size_t m_Width;
	size_t m_Height;
	size_t m_MipLevels;
	DXGI_FORMAT m_Format;

	Texture2DRenderBuffer();

	~Texture2DRenderBuffer();

	bool valid() const;

	operator ID3D11RenderTargetView* () const {return m_RTView;}
	operator ID3D11DepthStencilView* () const {return m_DSView;}
	operator ID3D11ShaderResourceView* () const {return m_SRView;}

	D3D11_VIEWPORT viewport();

	void create(ID3D11Device* d3dDevice,
		size_t width, size_t height, size_t mipLevels,
		DXGI_FORMAT dataFormat,
		DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1,
		DXGI_FORMAT srvFormat = (DXGI_FORMAT)-1);

	void destroy();

};	// Texture2DRenderBuffer

//! Texture2DArrayRenderBuffer
struct Texture2DArrayRenderBuffer
{
	ID3D11Texture2D* m_TextureObject;
	ID3D11RenderTargetView* m_RTView;
	ID3D11DepthStencilView* m_DSView;
	ID3D11ShaderResourceView* m_SRView;

	size_t m_Width;
	size_t m_Height;
	size_t m_ArraySize;
	size_t m_MipLevels;
	DXGI_FORMAT m_Format;

	Texture2DArrayRenderBuffer();

	~Texture2DArrayRenderBuffer();

	bool valid() const;

	operator ID3D11RenderTargetView* () const {return m_RTView;}
	operator ID3D11DepthStencilView* () const {return m_DSView;}
	operator ID3D11ShaderResourceView* () const {return m_SRView;}

	D3D11_VIEWPORT viewport();

	void create(
		ID3D11Device* d3dDevice,
		size_t width, size_t height, size_t arraySize, size_t mipLevels,
		DXGI_FORMAT dataFormat,
		DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1,
		DXGI_FORMAT srvFormat = (DXGI_FORMAT)-1);

	void destroy();

};	// Texture2DArrayRenderBuffer

}	// namespace js

#endif	// BUFFERS_H