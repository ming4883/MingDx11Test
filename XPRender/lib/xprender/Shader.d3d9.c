#include "Shader.d3d9.h"
#include "Texture.d3d9.h"
#include <stdio.h>

XprGpuShader* xprGpuShaderAlloc()
{
	XprGpuShader* self;
	XprAllocWithImpl(self, XprGpuShader, XprGpuShaderImpl);

	return self;
}

XprBool xprGpuShaderInit(XprGpuShader* self, const char** sources, size_t srcCnt, XprGpuShaderType type)
{
	HRESULT hr;
	ID3DXBuffer* code;
	ID3DXBuffer* errors;
	ID3DXConstantTable* constTable;

	// compile shader
	if(XprGpuShaderType_Vertex == type) {
		hr = D3DXCompileShader(sources[0], strlen(sources[0]),
			nullptr, nullptr, "main", D3DXGetVertexShaderProfile(xprAPI.d3ddev),
			0, &code, &errors, &constTable
			);
	}
	else if(XprGpuShaderType_Fragment == type) {
		hr = D3DXCompileShader(sources[0], strlen(sources[0]),
			nullptr, nullptr, "main", D3DXGetPixelShaderProfile(xprAPI.d3ddev),
			0, &code, &errors, &constTable
			);
	}
	else {
		xprDbgStr("d3d9 unsupported shader type %d", type);
		return XprFalse;
	}

	if(FAILED(hr)) {
		xprDbgStr("d3d9 failed to compile shader %s", errors->lpVtbl->GetBufferPointer(errors));
		errors->lpVtbl->Release(errors);
		return XprFalse;
	}

	// create shader
	if(XprGpuShaderType_Vertex == type) {
		hr = IDirect3DDevice9_CreateVertexShader(xprAPI.d3ddev,
			code->lpVtbl->GetBufferPointer(code),
			&self->impl->d3dvs
			);
	}
	else if(XprGpuShaderType_Fragment == type) {
		hr = IDirect3DDevice9_CreatePixelShader(xprAPI.d3ddev,
			code->lpVtbl->GetBufferPointer(code),
			&self->impl->d3dps
			);
	}

	if(FAILED(hr)) {
		xprDbgStr("d3d9 failed to create shader %8x", hr);
		code->lpVtbl->Release(code);
		constTable->lpVtbl->Release(constTable);
		return XprFalse;
	}

	// save the byte code for later use
	self->impl->bytecode = code;
	self->impl->constTable = constTable;

	self->flags |= XprGpuShader_Inited;

	return XprTrue;
}

void xprGpuShaderFree(XprGpuShader* self)
{
	if(nullptr == self)
		return;

	if(nullptr != self->impl->d3dvs) {
		IDirect3DVertexShader9_Release(self->impl->d3dvs);
	}
	if(nullptr != self->impl->d3dps) {
		IDirect3DPixelShader9_Release(self->impl->d3dps);
	}
	if(nullptr != self->impl->bytecode) {
		self->impl->bytecode->lpVtbl->Release(self->impl->bytecode);
	}
	if(nullptr != self->impl->constTable) {
		self->impl->constTable->lpVtbl->Release(self->impl->constTable);
	}

	free(self);
}

XprGpuProgram* xprGpuProgramAlloc()
{
	XprGpuProgram* self;
	XprAllocWithImpl(self, XprGpuProgram, XprGpuProgramImpl);

	return self;
}

void xprGpuProgramUniformCollect(XprGpuShader* self, XprGpuProgramUniform* table)
{
	UINT i;
	ID3DXConstantTable* constTable = self->impl->constTable;

	{
		const char* samplers[16] = {0};
		UINT samplerCnt = 0;
		
		D3DXGetShaderSamplers(
			self->impl->bytecode->lpVtbl->GetBufferPointer(self->impl->bytecode),
			samplers,
			&samplerCnt);

		for(i=0; i<samplerCnt; ++i) {
			D3DXHANDLE h = constTable->lpVtbl->GetConstantByName(constTable, nullptr, samplers[i]);
			XprGpuProgramUniform* uniform = malloc(sizeof(XprGpuProgramUniform));
			uniform->loc = 0;
			uniform->size = 0;
			uniform->hash = XprHash(samplers[i]);
			uniform->texunit = constTable->lpVtbl->GetSamplerIndex(constTable, h);

			HASH_ADD_INT(table, hash, uniform);
		}
	}
	{
		D3DXCONSTANTTABLE_DESC tblDesc;
		constTable->lpVtbl->GetDesc(constTable, &tblDesc);

		for(i=0; i<tblDesc.Constants; ++i) {
			XprGpuProgramUniform* uniform;
			D3DXHANDLE h = constTable->lpVtbl->GetConstant(constTable, nullptr, i);
			D3DXCONSTANT_DESC constDesc;
			UINT cnt = 1;
			if(D3D_OK != constTable->lpVtbl->GetConstantDesc(constTable, h, &constDesc, &cnt)) {
				continue;
			}

			uniform = malloc(sizeof(XprGpuProgramUniform));
			uniform->loc = constDesc.RegisterIndex;
			uniform->size = constDesc.RegisterCount;
			uniform->hash = XprHash(constDesc.Name);
			uniform->texunit = -1;

			HASH_ADD_INT(table, hash, uniform);

		}
	}


}

XprBool xprGpuProgramInit(XprGpuProgram* self, XprGpuShader** shaders, size_t shaderCnt)
{
	XprGpuShader* vs = nullptr;
	XprGpuShader* ps = nullptr;

	if(self->flags & XprGpuProgram_Inited) {
		xprDbgStr("XprGpuProgram already inited!\n");
		return XprFalse;
	}

#define COM_ADD_REF(x) x->lpVtbl->AddRef(x)

	// attach shaders
	{
		size_t i;
		for(i=0; i<shaderCnt; ++i) {
			if(nullptr == shaders[i])
				continue;
			
			switch(shaders[i]->type) {
				case XprGpuShaderType_Vertex:
					{
						vs = shaders[i];
						self->impl->d3dvs = shaders[i]->impl->d3dvs;
						COM_ADD_REF(self->impl->d3dvs);
						break;
					}
				case XprGpuShaderType_Fragment:
					{
						ps = shaders[i];
						self->impl->d3dps = shaders[i]->impl->d3dps;
						COM_ADD_REF(self->impl->d3dps);
						break;
					}
			}
		}
	}
#undef COM_ADD_REF

	if(nullptr == self->impl->d3dvs || nullptr == self->impl->d3dps) {
		xprDbgStr("d3d9 incomplete program vs:%d ps:%d", self->impl->d3dvs, self->impl->d3dps);
		return XprFalse;
	}

	// collect uniforms
	xprGpuProgramUniformCollect(vs, self->impl->uniformVs);
	xprGpuProgramUniformCollect(ps, self->impl->uniformPs);

	self->flags |= XprGpuProgram_Inited;

	return XprTrue;
	
}

void xprGpuProgramFree(XprGpuProgram* self)
{
	if(nullptr == self)
		return;

	{
		XprGpuProgramUniform* curr, *temp;
		HASH_ITER(hh, self->impl->uniformVs, curr, temp) {
			HASH_DEL(self->impl->uniformVs, curr);
			free(curr);
		}

		HASH_ITER(hh, self->impl->uniformPs, curr, temp) {
			HASH_DEL(self->impl->uniformPs, curr);
			free(curr);
		}
	}

	if(nullptr != self->impl->d3dvs) {
		IDirect3DVertexShader9_Release(self->impl->d3dvs);
	}

	if(nullptr != self->impl->d3dps) {
		IDirect3DPixelShader9_Release(self->impl->d3dps);
	}

	free(self);
}

void xprGpuProgramPreRender(XprGpuProgram* self)
{
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	IDirect3DDevice9_SetVertexShader(xprAPI.d3ddev, self->impl->d3dvs);
	IDirect3DDevice9_SetPixelShader(xprAPI.d3ddev, self->impl->d3dps);
}

XprBool xprGpuProgramUniformfv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}
	
	HASH_FIND_INT(self->impl->uniformVs, &hash, uniform);
	if(nullptr != uniform) {
		IDirect3DDevice9_SetVertexShaderConstantF(xprAPI.d3ddev, uniform->loc, value, uniform->size * count);
	}

	HASH_FIND_INT(self->impl->uniformPs, &hash, uniform);
	if(nullptr != uniform) {
		IDirect3DDevice9_SetPixelShaderConstantF(xprAPI.d3ddev, uniform->loc, value, uniform->size * count);
	}
	
	return XprTrue;
}


XprBool xprGpuProgramUniform1fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	return xprGpuProgramUniformfv(self, hash, count, value);
}

XprBool xprGpuProgramUniform2fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	return xprGpuProgramUniformfv(self, hash, count, value);
}

XprBool xprGpuProgramUniform3fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	return xprGpuProgramUniformfv(self, hash, count, value);
}

XprBool xprGpuProgramUniform4fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	return xprGpuProgramUniformfv(self, hash, count, value);
}

XprBool xprGpuProgramUniformMtx4fv(XprGpuProgram* self, XprHashCode hash, size_t count, XprBool transpose, const float* value)
{
	return xprGpuProgramUniformfv(self, hash, count, value);
}

XprBool xprGpuProgramUniformTexture(XprGpuProgram* self, XprHashCode hash, struct XprTexture* texture)
{
	XprGpuProgramUniform* uniform;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}
	
	HASH_FIND_INT(self->impl->uniformVs, &hash, uniform);
	if(nullptr != uniform && -1 != uniform->texunit) {
		IDirect3DDevice9_SetTexture(xprAPI.d3ddev, uniform->texunit, (IDirect3DBaseTexture9*)texture->impl->d3dtex);
	}

	HASH_FIND_INT(self->impl->uniformPs, &hash, uniform);
	if(nullptr != uniform) {
		IDirect3DDevice9_SetTexture(xprAPI.d3ddev, uniform->texunit, (IDirect3DBaseTexture9*)texture->impl->d3dtex);
	}
	
	return XprTrue;
}