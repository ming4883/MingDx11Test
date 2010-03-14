#include "DXUT.h"
#include "Shaders.h"
#include <string>

namespace js
{
	bool Shaders::compileFromFile(
		const wchar_t* filePath,
		const char* entryPoint,
		const char* shaderModel,
		ID3DBlob*& outByteCode)
	{
		HRESULT hr;

		DWORD flags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3D10_SHADER_DEBUG;
#endif

		ID3DBlob* errors;

		static D3D10_SHADER_MACRO* macros = nullptr;
		static ID3D10Include* include = nullptr;

		hr = D3DX11CompileFromFileW(
			filePath,
			macros,
			include,
			entryPoint, shaderModel,
			flags,
			0,
			nullptr,
			&outByteCode,
			&errors,
			nullptr);

		if(FAILED(hr))
		{
			if(errors != nullptr)
				OutputDebugStringA((char*)errors->GetBufferPointer());
			js_safe_release(errors);
			return false;
		}

		js_safe_release(errors);

		return true;
	}
	
	bool Shaders::compile(
		const char* srcData,
		const size_t srcSize,
		const char* srcName,
		const char* entryPoint,
		const char* shaderModel,
		ID3DBlob*& outByteCode)
	{
		HRESULT hr;

		DWORD flags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3D10_SHADER_DEBUG;
#endif

		ID3DBlob* errors;

		static D3D10_SHADER_MACRO* macros = nullptr;
		static ID3D10Include* include = nullptr;

		hr = D3DCompile(
			srcData, srcSize, srcName,
			macros, include,
			entryPoint, shaderModel,
			flags, 0,
			&outByteCode, &errors);

		if(FAILED(hr))
		{
			if(errors != nullptr)
				OutputDebugStringA((char*)errors->GetBufferPointer());
			js_safe_release(errors);
			return false;
		}

		js_safe_release(errors);

		return true;
	}

	inline std::string getBestShaderModel(D3D_FEATURE_LEVEL featureLevel)
	{
		switch(featureLevel)
		{
			case D3D_FEATURE_LEVEL_11_0: return "_5_0";
			case D3D_FEATURE_LEVEL_10_1: return "_4_1";
			case D3D_FEATURE_LEVEL_10_0: return "_4_0";
			case D3D_FEATURE_LEVEL_9_3:  return "_4_0_level_9_3";
			case D3D_FEATURE_LEVEL_9_2:
			case D3D_FEATURE_LEVEL_9_1:  return "_4_0_level_9_1";
		}

		return "_4_0_level_9_1";
	}

	ID3D11VertexShader* Shaders::createFromFileVertex(
		ID3D11Device* d3dDevice,
		const wchar_t* filePath,
		const char* entryPoint,
		ID3DBlob** outByteCode)
	{
		HRESULT hr;

		js_assert(d3dDevice != nullptr);

		const std::string shaderModel = "vs" + getBestShaderModel(d3dDevice->GetFeatureLevel());

		ID3DBlob* byteCode;

		if(!compileFromFile(filePath, entryPoint, shaderModel.c_str(), byteCode))
			return nullptr;

		ID3D11VertexShader* shader = nullptr;

		hr = d3dDevice->CreateVertexShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(),  nullptr, &shader);

		if(FAILED(hr))
		{
			js_safe_release(byteCode);
			return nullptr;
		}

		if(nullptr != outByteCode)
			*outByteCode = byteCode;
		
		return shader;
	}

	ID3D11GeometryShader* Shaders::createFromFileGeometry(
		ID3D11Device* d3dDevice,
		const wchar_t* filePath,
		const char* entryPoint,
		ID3DBlob** outByteCode)
	{
		HRESULT hr;

		js_assert(d3dDevice != nullptr);

		const std::string shaderModel = "gs" + getBestShaderModel(d3dDevice->GetFeatureLevel());

		ID3DBlob* byteCode;

		if(!compileFromFile(filePath, entryPoint, shaderModel.c_str(), byteCode))
			return nullptr;

		ID3D11GeometryShader* shader = nullptr;

		hr = d3dDevice->CreateGeometryShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(),  nullptr, &shader);

		if(FAILED(hr))
		{
			js_safe_release(byteCode);
			return nullptr;
		}

		if(nullptr != outByteCode)
			*outByteCode = byteCode;
		
		return shader;
	}
	ID3D11PixelShader* Shaders::createFromFilePixel(
		ID3D11Device* d3dDevice,
		const wchar_t* filePath,
		const char* entryPoint,
		ID3DBlob** outByteCode)
	{
		HRESULT hr;

		js_assert(d3dDevice != nullptr);

		const std::string shaderModel = "ps" + getBestShaderModel(d3dDevice->GetFeatureLevel());

		ID3DBlob* byteCode;

		if(!compileFromFile(filePath, entryPoint, shaderModel.c_str(), byteCode))
			return nullptr;

		ID3D11PixelShader* shader = nullptr;

		hr = d3dDevice->CreatePixelShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(),  nullptr, &shader);

		if(FAILED(hr))
		{
			js_safe_release(byteCode);
			return nullptr;
		}

		if(nullptr != outByteCode)
			*outByteCode = byteCode;
		
		return shader;
	}

}	// namespace js