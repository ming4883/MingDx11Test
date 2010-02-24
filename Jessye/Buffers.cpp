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

}	// namespace js