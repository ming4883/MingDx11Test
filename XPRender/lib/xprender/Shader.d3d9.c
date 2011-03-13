#include "Shader.d3d9.h"
#include "Texture.d3d9.h"
#include "Buffer.d3d9.h"
#include <stdio.h>

XprGpuShader* xprGpuShaderAlloc()
{
	XprGpuShaderImpl* self = malloc(sizeof(XprGpuShaderImpl));
	memset(self, 0, sizeof(XprGpuShaderImpl));
	return &self->i;
}

XprBool xprGpuShaderInit(XprGpuShader* self, const char** sources, size_t srcCnt, XprGpuShaderType type)
{
	HRESULT hr;
	ID3DXBuffer* code;
	ID3DXBuffer* errors;
	ID3DXConstantTable* constTable;
	const char* profile = nullptr;
	XprGpuShaderImpl* impl = (XprGpuShaderImpl*)self;

	// compile shader
	if(XprGpuShaderType_Vertex == type) {
		profile = D3DXGetVertexShaderProfile(xprAPI.d3ddev);
	}
	else if(XprGpuShaderType_Fragment == type) {
		profile = D3DXGetPixelShaderProfile(xprAPI.d3ddev);
	}
	else {
		xprDbgStr("d3d9 unsupported shader type %d", type);
		return XprFalse;
	}

	hr = D3DXCompileShader(sources[0], strlen(sources[0]),
		nullptr, nullptr, "main", profile,
		0, &code, &errors, &constTable
		);

	if(FAILED(hr)) {
		xprDbgStr("d3d9 failed to compile shader\n%s", errors->lpVtbl->GetBufferPointer(errors));
		errors->lpVtbl->Release(errors);
		{
			FILE* fp = fopen("error-shader.txt", "w");
			fprintf(fp, "%s", sources[0]);
			fclose(fp);
		}
		return XprFalse;
	}

	// create shader
	if(XprGpuShaderType_Vertex == type) {
		hr = IDirect3DDevice9_CreateVertexShader(xprAPI.d3ddev,
			code->lpVtbl->GetBufferPointer(code),
			&impl->d3dvs
			);
	}
	else if(XprGpuShaderType_Fragment == type) {
		hr = IDirect3DDevice9_CreatePixelShader(xprAPI.d3ddev,
			code->lpVtbl->GetBufferPointer(code),
			&impl->d3dps
			);
	}

	if(FAILED(hr)) {
		xprDbgStr("d3d9 failed to create shader %8x", hr);
		
		constTable->lpVtbl->Release(constTable);
		return XprFalse;
	}
	
	code->lpVtbl->Release(code);

	// save the byte code for later use
	//impl->bytecode = code;
	impl->constTable = constTable;

	self->type = type;
	self->flags |= XprGpuShader_Inited;

	return XprTrue;
}

void xprGpuShaderFree(XprGpuShader* self)
{
	XprGpuShaderImpl* impl = (XprGpuShaderImpl*)self;

	if(nullptr == self)
		return;

	if(nullptr != impl->d3dvs) {
		IDirect3DVertexShader9_Release(impl->d3dvs);
	}
	if(nullptr != impl->d3dps) {
		IDirect3DPixelShader9_Release(impl->d3dps);
	}
	//if(nullptr != impl->bytecode) {
	//	impl->bytecode->lpVtbl->Release(impl->bytecode);
	//}
	if(nullptr != impl->constTable) {
		impl->constTable->lpVtbl->Release(impl->constTable);
	}

	free(self);
}

XprGpuProgram* xprGpuProgramAlloc()
{
	XprGpuProgramImpl* self = malloc(sizeof(XprGpuProgramImpl));
	memset(self, 0, sizeof(XprGpuProgramImpl));
	return &self->i;
}

void xprGpuProgramUniformCollect(XprGpuShader* self, XprGpuProgramUniform** table)
{
	UINT i;
	ID3DXConstantTable* constTable = ((XprGpuShaderImpl*)self)->constTable;
	
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

			if(D3DXRS_SAMPLER == constDesc.RegisterSet) {
				uniform->texunit = constTable->lpVtbl->GetSamplerIndex(constTable, h);
			}
			else {
				uniform->texunit = -1;
			}
			HASH_ADD_INT(*table, hash, uniform);
		}
	}
}

XprBool xprGpuProgramInit(XprGpuProgram* self, XprGpuShader** shaders, size_t shaderCnt)
{
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;
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
						impl->d3dvs = ((XprGpuShaderImpl*)shaders[i])->d3dvs;
						COM_ADD_REF(impl->d3dvs);
						break;
					}
				case XprGpuShaderType_Fragment:
					{
						ps = shaders[i];
						impl->d3dps = ((XprGpuShaderImpl*)shaders[i])->d3dps;
						COM_ADD_REF(impl->d3dps);
						break;
					}
			}
		}
	}
#undef COM_ADD_REF

	if(nullptr == impl->d3dvs || nullptr == impl->d3dps) {
		xprDbgStr("d3d9 incomplete program vs:%d ps:%d", impl->d3dvs, impl->d3dps);
		return XprFalse;
	}

	// collect uniforms
	xprGpuProgramUniformCollect(vs, &impl->uniformVs);
	xprGpuProgramUniformCollect(ps, &impl->uniformPs);

	self->flags |= XprGpuProgram_Inited;

	return XprTrue;
	
}

void xprGpuProgramFree(XprGpuProgram* self)
{
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return;

	{
		XprGpuProgramUniform* curr, *temp;
		HASH_ITER(hh, impl->uniformVs, curr, temp) {
			HASH_DEL(impl->uniformVs, curr);
			free(curr);
		}

		HASH_ITER(hh, impl->uniformPs, curr, temp) {
			HASH_DEL(impl->uniformPs, curr);
			free(curr);
		}
	}

	if(nullptr != impl->d3dvs) {
		IDirect3DVertexShader9_Release(impl->d3dvs);
	}

	if(nullptr != impl->d3dps) {
		IDirect3DPixelShader9_Release(impl->d3dps);
	}

	{
		XprGpuProgramInputAssembly* curr, *temp;
		HASH_ITER(hh, impl->ias, curr, temp) {
			HASH_DEL(impl->ias, curr);
			IDirect3DVertexDeclaration9_Release(curr->d3ddecl);
			free(curr);
		}
	}

	free(self);
}

void xprGpuProgramPreRender(XprGpuProgram* self)
{
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	IDirect3DDevice9_SetVertexShader(xprAPI.d3ddev, impl->d3dvs);
	IDirect3DDevice9_SetPixelShader(xprAPI.d3ddev, impl->d3dps);
}

XprBool xprGpuProgramUniformfv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}
	
	HASH_FIND_INT(impl->uniformVs, &hash, uniform);
	if(nullptr != uniform) {
		IDirect3DDevice9_SetVertexShaderConstantF(xprAPI.d3ddev, uniform->loc, value, uniform->size * count);
	}

	HASH_FIND_INT(impl->uniformPs, &hash, uniform);
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

static D3DTEXTUREFILTERTYPE xprD3D9_SAMPLER_MAG_FILTER[] = {
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
};

static D3DTEXTUREFILTERTYPE xprD3D9_SAMPLER_MIN_FILTER[] = {
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
};

static D3DTEXTUREFILTERTYPE xprD3D9_SAMPLER_MIP_FILTER[] = {
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_LINEAR,
	D3DTEXF_POINT,
	D3DTEXF_NONE,
	D3DTEXF_NONE,
};

static D3DTEXTUREADDRESS xprD3D9_SAMPLER_ADDRESS[] = {
	D3DTADDRESS_WRAP,
	D3DTADDRESS_CLAMP,
};

XprBool xprGpuProgramUniformTexture(XprGpuProgram* self, XprHashCode hash, struct XprTexture* texture, const struct XprSampler* sampler)
{
	XprGpuProgramUniform* uniform;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}
	
	HASH_FIND_INT(impl->uniformVs, &hash, uniform);
	if(nullptr != uniform && -1 != uniform->texunit) {
		IDirect3DDevice9_SetTexture(xprAPI.d3ddev, uniform->texunit, (IDirect3DBaseTexture9*)((XprTextureImpl*)texture)->d3dtex);
	}

	HASH_FIND_INT(impl->uniformPs, &hash, uniform);
	if(nullptr != uniform) {
		IDirect3DDevice9_SetTexture(xprAPI.d3ddev, uniform->texunit, (IDirect3DBaseTexture9*)((XprTextureImpl*)texture)->d3dtex);
		IDirect3DDevice9_SetSamplerState(xprAPI.d3ddev, uniform->texunit, D3DSAMP_MAGFILTER, xprD3D9_SAMPLER_MAG_FILTER[sampler->filter]);
		IDirect3DDevice9_SetSamplerState(xprAPI.d3ddev, uniform->texunit, D3DSAMP_MINFILTER, xprD3D9_SAMPLER_MIN_FILTER[sampler->filter]);
		IDirect3DDevice9_SetSamplerState(xprAPI.d3ddev, uniform->texunit, D3DSAMP_MIPFILTER, xprD3D9_SAMPLER_MIP_FILTER[sampler->filter]);
		IDirect3DDevice9_SetSamplerState(xprAPI.d3ddev, uniform->texunit, D3DSAMP_ADDRESSU, xprD3D9_SAMPLER_ADDRESS[sampler->addressU]);
		IDirect3DDevice9_SetSamplerState(xprAPI.d3ddev, uniform->texunit, D3DSAMP_ADDRESSV, xprD3D9_SAMPLER_ADDRESS[sampler->addressV]);
		IDirect3DDevice9_SetSamplerState(xprAPI.d3ddev, uniform->texunit, D3DSAMP_ADDRESSW, xprD3D9_SAMPLER_ADDRESS[sampler->addressW]);
	}
	
	return XprTrue;
}


XprInputGpuFormatMapping XprInputGpuFormatMappings[] = {
	{XprGpuFormat_FloatR32,				D3DDECLTYPE_FLOAT1, sizeof(float)},
	{XprGpuFormat_FloatR32G32,			D3DDECLTYPE_FLOAT2, sizeof(float) * 2},
	{XprGpuFormat_FloatR32G32B32,		D3DDECLTYPE_FLOAT3, sizeof(float) * 3},
	{XprGpuFormat_FloatR32G32B32A32,	D3DDECLTYPE_FLOAT4, sizeof(float) * 4},
};

XprInputGpuFormatMapping* xprInputGpuFormatMappingGet(XprGpuFormat xprFormat)
{
	size_t i=0;
	for(i=0; i<xprCountOf(XprInputGpuFormatMappings); ++i) {
		XprInputGpuFormatMapping* mapping = &XprInputGpuFormatMappings[i];
		if(xprFormat == mapping->xprFormat)
			return mapping;
	}

	return nullptr;
}

D3DDECLUSAGE xprGpuInputGetUsage(XprGpuProgramInput* input)
{
	if(strstr(input->name, "pos") || strstr(input->name, "vertex"))
		return D3DDECLUSAGE_POSITION;

	if(strstr(input->name, "nor"))
		return D3DDECLUSAGE_NORMAL;

	if(strstr(input->name, "col"))
		return D3DDECLUSAGE_COLOR;
	
	if(strstr(input->name, "tex"))
		return D3DDECLUSAGE_TEXCOORD;

	return D3DDECLUSAGE_POSITION;
}

BYTE xprGpuInputGetUsageIndex(XprGpuProgramInput* input)
{
	size_t len = strlen(input->name);
	char ch;

	if(0 == len)
		return 0;

	ch = input->name[len-1];

	if(ch < '0' || ch > '9')
		return 0;

	return ch - '0';
}

void xprGpuProgramBindBuffer(XprGpuProgram* self, XprGpuProgramInput* inputs, size_t count)
{
	size_t i = 0;
	size_t stream = 0;
	XprBufferImpl* lastBuffer = nullptr;

	for(i=0; i<count; ++i) {
		XprGpuProgramInput* input = &inputs[i];
		XprBufferImpl* buffer = (XprBufferImpl*)input->buffer;
		XprInputGpuFormatMapping* m = xprInputGpuFormatMappingGet(input->format);

		if(nullptr == buffer)
			continue;

		if(XprBufferType_Index == buffer->i.type) {
			// bind index buffer
			IDirect3DDevice9_SetIndices(xprAPI.d3ddev, buffer->d3dib);
		}
		else if(XprBufferType_Vertex == buffer->i.type) {
			// bind vertex buffer
			if(lastBuffer == buffer) 
				continue;

			if(nullptr != lastBuffer)
				++stream;

			IDirect3DDevice9_SetStreamSource(xprAPI.d3ddev, stream, buffer->d3dvb, 0, m->stride);
			lastBuffer = buffer;
		}
	}
}

static D3DVERTEXELEMENT9 xprD3D9_ELEM_END = D3DDECL_END();
static D3DVERTEXELEMENT9 xprD3D9_ELEMS[16];

void xprGpuProgramBindVertexDecl(XprGpuProgram* self, size_t gpuInputId, XprGpuProgramInput* inputs, size_t count)
{
	XprGpuProgramInputAssembly* ia;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	HASH_FIND_INT(impl->ias, &gpuInputId, ia);
	if(nullptr == ia) {

		size_t i = 0;
		size_t stream = 0;
		size_t offset = 0;
		size_t elem = 0;
		XprBufferImpl* lastBuffer = nullptr;

		for(i=0; i<count; ++i) {
			XprGpuProgramInput* input = &inputs[i];
			XprBufferImpl* buffer = (XprBufferImpl*)input->buffer;
			XprInputGpuFormatMapping* m = xprInputGpuFormatMappingGet(input->format);

			if(nullptr == buffer)
				continue;

			if(XprBufferType_Vertex != buffer->i.type)
				continue;

			if(lastBuffer != buffer) {
				if(nullptr != lastBuffer) {
					++stream;
					offset = 0;
				}
				lastBuffer = buffer;
			}

			xprD3D9_ELEMS[elem].Stream = stream;
			xprD3D9_ELEMS[elem].Offset = offset;
			xprD3D9_ELEMS[elem].Type = m->declType;
			xprD3D9_ELEMS[elem].Method = D3DDECLMETHOD_DEFAULT;
			xprD3D9_ELEMS[elem].Usage = xprGpuInputGetUsage(input);
			xprD3D9_ELEMS[elem].UsageIndex = xprGpuInputGetUsageIndex(input);

			++elem;
			offset += m->stride;
		}
		xprD3D9_ELEMS[elem] = xprD3D9_ELEM_END;

		ia = malloc(sizeof(XprGpuProgramInputAssembly));
		ia->gpuInputId = gpuInputId;
		IDirect3DDevice9_CreateVertexDeclaration(xprAPI.d3ddev, xprD3D9_ELEMS, &ia->d3ddecl);

		HASH_ADD_INT(impl->ias, gpuInputId, ia);
	}

	IDirect3DDevice9_SetVertexDeclaration(xprAPI.d3ddev, ia->d3ddecl);
}

size_t xprGenGpuInputId()
{
	return ++xprAPI.gpuInputId;
}

void xprGpuProgramBindInput(XprGpuProgram* self, size_t gpuInputId, XprGpuProgramInput* inputs, size_t count)
{
	xprGpuProgramBindBuffer(self, inputs, count);
	xprGpuProgramBindVertexDecl(self, gpuInputId, inputs, count);
}

void xprGpuDrawPoint(size_t offset, size_t count)
{
	IDirect3DDevice9_DrawPrimitive(xprAPI.d3ddev, D3DPT_POINTLIST, offset, count);
}

void xprGpuDrawLine(size_t offset, size_t count, size_t flags)
{
	D3DPRIMITIVETYPE mode = (flags & XprGpuDraw_Stripped) ? D3DPT_LINESTRIP : D3DPT_LINELIST;
	IDirect3DDevice9_DrawPrimitive(xprAPI.d3ddev, mode, offset, count / 2);
}

void xprGpuDrawLineIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags)
{
	D3DPRIMITIVETYPE mode = (flags & XprGpuDraw_Stripped) ? D3DPT_LINESTRIP : D3DPT_LINELIST;
	IDirect3DDevice9_DrawIndexedPrimitive(xprAPI.d3ddev, mode, 0, minIdx, maxIdx+1, offset, count / 2);
}

void xprGpuDrawTriangle(size_t offset, size_t count, size_t flags)
{
	D3DPRIMITIVETYPE mode = (flags & XprGpuDraw_Stripped) ? D3DPT_TRIANGLESTRIP : D3DPT_TRIANGLELIST;
	IDirect3DDevice9_DrawPrimitive(xprAPI.d3ddev, mode, offset, count / 3);
}

void xprGpuDrawTriangleIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags)
{
	D3DPRIMITIVETYPE mode = (flags & XprGpuDraw_Stripped) ? D3DPT_TRIANGLESTRIP : D3DPT_TRIANGLELIST;
	IDirect3DDevice9_DrawIndexedPrimitive(xprAPI.d3ddev, mode, 0, minIdx, maxIdx+1, offset, count / 3);
}

void xprGpuDrawPatch(size_t offset, size_t count, size_t vertexPerPatch, size_t flags)
{
	// not supported
}

void xprGpuDrawPatchIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t vertexPerPatch, size_t flags)
{
	// not supported
}
