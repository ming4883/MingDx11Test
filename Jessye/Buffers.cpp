#include "DXUT.h"
#include "Buffers.h"
#include "Shaders.h"

#include <sstream>

namespace js
{
	ID3D11InputLayout* Buffers::createInputLayout(
		ID3D11Device* d3dDevice,
		const D3D11_INPUT_ELEMENT_DESC* inputElements,
		const size_t inputElementCount,
		const VertexShader& shader)
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

		if(!shader.valid())
			return nullptr;

		ID3D11InputLayout* layout = nullptr;
		HRESULT hr = d3dDevice->CreateInputLayout(
			inputElements, inputElementCount,
			shader.m_ByteCode->GetBufferPointer(), shader.m_ByteCode->GetBufferSize(),
			&layout);

		if(FAILED(hr)) js_safe_release(layout);

		return layout;
	}

	ID3D11Buffer* Buffers::createConstantBuffer(
		ID3D11Device* d3dDevice,
		size_t sizeInBytes)
	{
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

	ID3D11Texture2D* Buffers::createTexture2DRenderBuffer(
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		DXGI_FORMAT format,
		size_t mipLevels)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.Format = format;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = mipLevels;
		desc.SampleDesc.Count = 1;

		ID3D11Texture2D* texture = nullptr;
		HRESULT hr = d3dDevice->CreateTexture2D(&desc, nullptr, &texture);

		if(FAILED(hr)) js_safe_release(texture);

		return texture;
	}

	ID3D11RenderTargetView* Buffers::createRenderTargetView(
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		size_t mipLevel)
	{
		D3D11_TEXTURE2D_DESC texdesc;
		texture->GetDesc(&texdesc);

		D3D11_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = texdesc.Format;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mipLevel;

		ID3D11RenderTargetView* rtview = nullptr;
		HRESULT hr = d3dDevice->CreateRenderTargetView(texture, &desc, &rtview);

		if(FAILED(hr)) js_safe_release(rtview);

		return rtview;
	}

	ID3D11ShaderResourceView* Buffers::createShaderResourceView(
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture)
	{
		D3D11_TEXTURE2D_DESC texdesc;
		texture->GetDesc(&texdesc);

		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = texdesc.Format;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = texdesc.MipLevels;
		desc.Texture2D.MostDetailedMip = 0;

		ID3D11ShaderResourceView* srview = nullptr;
		HRESULT hr = d3dDevice->CreateShaderResourceView(texture, &desc, &srview);

		if(FAILED(hr)) js_safe_release(srview);

		return srview;
	}

}	// namespace js