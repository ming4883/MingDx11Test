#ifndef __XPRENDER_SHADER_D3D9_H__
#define __XPRENDER_SHADER_D3D9_H__

#include "API.d3d9.h"
#include "Shader.h"
#include "uthash/uthash.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprGpuShaderImpl
{
	XprGpuShader i;

	IDirect3DVertexShader9* d3dvs;
	IDirect3DPixelShader9* d3dps;
	ID3DXBuffer* bytecode;
	ID3DXConstantTable* constTable;

} XprGpuShaderImpl;

typedef struct XprGpuProgramUniform
{
	XprHashCode hash;
	UINT loc;
	UINT size;
	int texunit;
	UT_hash_handle hh;

} XprGpuProgramUniform;

typedef struct XprGpuProgramImpl
{
	XprGpuProgram i;

	IDirect3DVertexShader9* d3dvs;
	IDirect3DPixelShader9* d3dps;
	XprGpuProgramUniform* uniformVs;
	XprGpuProgramUniform* uniformPs;

} XprGpuProgramImpl;

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_D3D9_H__