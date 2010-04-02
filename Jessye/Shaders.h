#ifndef SHADERS_H
#define SHADERS_H

#include "Platform.h"

namespace js
{

struct Shaders
{
	static bool compileFromFile(
		const wchar_t* filePath,
		const char* entryPoint,
		const char* shaderModel,
		ID3DBlob*& outByteCode);
	
	static bool compile(
		const char* srcData,
		const size_t srcSize,
		const char* srcName,
		const char* entryPoint,
		const char* shaderModel,
		ID3DBlob*& outByteCode);

	static ID3D11VertexShader* createFromFileVertex(
		ID3D11Device* d3dDevice,
		const wchar_t* filePath,
		const char* entryPoint,
		ID3DBlob** outByteCode = nullptr);
	
	static ID3D11GeometryShader* createFromFileGeometry(
		ID3D11Device* d3dDevice,
		const wchar_t* filePath,
		const char* entryPoint,
		ID3DBlob** outByteCode = nullptr);

	static ID3D11PixelShader* createFromFilePixel(
		ID3D11Device* d3dDevice,
		const wchar_t* filePath,
		const char* entryPoint,
		ID3DBlob** outByteCode = nullptr);

};	// Shaders

template<typename T>
struct Shader_t
{
	T* m_ShaderObject;
	ID3DBlob* m_ByteCode;

	Shader_t()
		: m_ByteCode(nullptr)
		, m_ShaderObject(nullptr)
	{
	}

	virtual ~Shader_t()
	{
		destroy();
	}

	bool valid() const
	{
		return m_ShaderObject != nullptr;
	}

	void destroy()
	{
		js_safe_release(m_ByteCode);
		js_safe_release(m_ShaderObject);
	}

	operator T* () const { return m_ShaderObject; }

};	// Shader_t

struct VertexShader : public Shader_t<ID3D11VertexShader>
{
public:
	void createFromFile(
		ID3D11Device* d3dDevice,
		const wchar_t* filePath,
		const char* entryPoint)
	{
		m_ShaderObject = Shaders::createFromFileVertex(d3dDevice, filePath, entryPoint, &m_ByteCode);
	}
};	// VertexShader

struct GeometryShader : public Shader_t<ID3D11GeometryShader>
{
public:
	void createFromFile(
		ID3D11Device* d3dDevice,
		const wchar_t* filePath,
		const char* entryPoint)
	{
		m_ShaderObject = Shaders::createFromFileGeometry(d3dDevice, filePath, entryPoint, &m_ByteCode);
	}
};	// GeometryShader

struct PixelShader : public Shader_t<ID3D11PixelShader>
{
public:
	void createFromFile(
		ID3D11Device* d3dDevice,
		const wchar_t* filePath,
		const char* entryPoint)
	{
		m_ShaderObject = Shaders::createFromFilePixel(d3dDevice, filePath, entryPoint, &m_ByteCode);
	}
};	// PixelShader


}	// namespace js

#endif	// SHADERS_H