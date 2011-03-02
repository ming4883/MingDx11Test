#include "GpuState.d3d9.h"

XprGpuState* xprGpuStateAlloc()
{
	XprGpuStateImpl* self = malloc(sizeof(XprGpuStateImpl));
	memset(self, 0, sizeof(XprGpuStateImpl));
	return &self->i;
}

void xprGpuStateFree(XprGpuState* self)
{
	if(nullptr == self)
		return;

	free(self);
}

void xprGpuStateInit(XprGpuState* self)
{
	self->desc.depthTest = XprTrue;
	self->desc.depthWrite = XprTrue;
	self->desc.cull = XprTrue;
	self->desc.blend = XprFalse;
	self->desc.blendFactorSrcRGB = XprGpuState_BlendFactor_One;
	self->desc.blendFactorDestRGB = XprGpuState_BlendFactor_Zero;
	self->desc.blendFactorSrcA = XprGpuState_BlendFactor_One;
	self->desc.blendFactorDestA = XprGpuState_BlendFactor_Zero;
	self->desc.polygonMode = XprGpuState_PolygonMode_Fill;

	self->flags = XprGpuState_Inited;
}

static D3DBLEND XprGpuState_blendFactorMapping[] = {
	D3DBLEND_ONE,
	D3DBLEND_ZERO,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_INVSRCCOLOR,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_INVDESTCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA,
};

static D3DFILLMODE XprGpuState_polygonModeMapping[] = {
	D3DFILL_WIREFRAME,
	D3DFILL_SOLID,
};

void xprGpuStatePreRender(XprGpuState* self)
{
	IDirect3DDevice9* d3ddev = xprAPI.d3ddev;

	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ZENABLE, (BOOL)self->desc.depthTest);
	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ZWRITEENABLE, (BOOL)self->desc.depthWrite);
	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_CULLMODE, self->desc.cull ? D3DCULL_CW : D3DCULL_NONE);
	
	if(self->desc.blend) {
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_SRCBLEND, XprGpuState_blendFactorMapping[self->desc.blendFactorSrcRGB - XprGpuState_BlendFactor_One]);
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_DESTBLEND, XprGpuState_blendFactorMapping[self->desc.blendFactorDestRGB - XprGpuState_BlendFactor_One]);
		
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_SRCBLENDALPHA, XprGpuState_blendFactorMapping[self->desc.blendFactorSrcA - XprGpuState_BlendFactor_One]);
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_DESTBLENDALPHA, XprGpuState_blendFactorMapping[self->desc.blendFactorDestA - XprGpuState_BlendFactor_One]);
		
	}
	else {
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ALPHABLENDENABLE, FALSE);
	}

	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_FILLMODE, XprGpuState_polygonModeMapping[self->desc.polygonMode - XprGpuState_PolygonMode_Line]);
}
