#ifndef BUFFERS_H
#define BUFFERS_H

#include "Platform.h"

namespace js
{

struct VertexShader;

struct Buffers
{
	static ID3D11InputLayout* createInputLayout(
		ID3D11Device* d3dDevice,
		const D3D11_INPUT_ELEMENT_DESC* inputElements,
		const size_t inputElementCount,
		const VertexShader& shader);

	static ID3D11Buffer* createConstantBuffer(
		ID3D11Device* d3dDevice,
		size_t sizeInBytes);

	static ID3D11Texture2D* createTexture2DRenderBuffer(
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		DXGI_FORMAT format,
		size_t mipLevels);

	static ID3D11RenderTargetView* createRenderTargetView(
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		size_t mipLevel);

	static ID3D11ShaderResourceView* createShaderResourceView(
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture);
		
};	// Buffers

template<typename T>
struct ConstantBuffer_t
{
	ID3D11Buffer* m_BufferObject;
	void* m_MappedPtr;

	ConstantBuffer_t() : m_BufferObject(nullptr), m_MappedPtr(nullptr)
	{
	}

	~ConstantBuffer_t()
	{
		js_safe_release(m_BufferObject);
	}

	bool valid() const
	{
		return m_BufferObject != nullptr;
	}

	void create(ID3D11Device* d3dDevice)
	{
		m_BufferObject = Buffers::createConstantBuffer(d3dDevice, sizeof(T));
	}

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

		d3dContext->Unmap(m_BufferObject, 0);
		m_MappedPtr = nullptr;
	}

	T& data()
	{
		js_assert(nullptr != m_MappedPtr);
		return *static_cast<T*>(m_MappedPtr);
	}

};

}	// namespace js

#endif	// BUFFERS_H