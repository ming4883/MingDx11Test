#include "GpuState.gl.h"

XprGpuState* xprGpuStateAlloc()
{
	XprGpuState* self;
	XprAllocWithImpl(self, XprGpuState, XprGpuStateImpl);

	return self;
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

static GLenum XprGpuState_blendFactorMapping[] = {
	GL_ONE,
	GL_ZERO,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
};

#if !defined(XPR_GLES_2)
static GLenum XprGpuState_polygonModeMapping[] = {
	GL_LINE,
	GL_FILL,
};
#endif

void xprGpuStatePreRender(XprGpuState* self)
{
	if(self->desc.depthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if(self->desc.depthWrite)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	if(self->desc.cull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if(self->desc.blend) {
		glEnable(GL_BLEND);
		
		glBlendFuncSeparate(
			XprGpuState_blendFactorMapping[self->desc.blendFactorSrcRGB - XprGpuState_BlendFactor_One],
			XprGpuState_blendFactorMapping[self->desc.blendFactorDestRGB - XprGpuState_BlendFactor_One],
			XprGpuState_blendFactorMapping[self->desc.blendFactorSrcA - XprGpuState_BlendFactor_One],
			XprGpuState_blendFactorMapping[self->desc.blendFactorDestA - XprGpuState_BlendFactor_One]);
	}
	else {
		glDisable(GL_BLEND);
	}

#if !defined(XPR_GLES_2)
	glPolygonMode(GL_FRONT_AND_BACK, XprGpuState_polygonModeMapping[self->desc.polygonMode - XprGpuState_PolygonMode_Line]);
#endif
}

/*
void xprGpuStateSetDepthTestEnabled(XprGpuState* self, XprBool value)
{
	if(nullptr == self)
		return;

	self->desc.impl->depthTest = value;
}

void xprGpuStateSetDepthWriteEnabled(XprGpuState* self, XprBool value)
{
	if(nullptr == self)
		return;

	self->desc.impl->depthWrite = value;
}

void xprGpuStateSetCullEnabled(XprGpuState* self, XprBool value)
{
	if(nullptr == self)
		return;

	self->desc.impl->culling = value;
}

void xprGpuStateSetBlendEnabled(XprGpuState* self, XprBool value)
{
	if(nullptr == self)
		return;

	self->desc.impl->blending = value;
}

void xprGpuStateSetBlendFactorRGB(XprGpuState* self, XprGpuStateType blendFactorSrc, XprGpuStateType blendFactorDest)
{
	if(nullptr == self)
		return;

	self->desc.impl->blendFactorSrc = blendFactorSrc;
	self->desc.impl->blendFactorDest = blendFactorDest;
}

void xprGpuStateSetBlendFactorA(XprGpuState* self, XprGpuStateType blendFactorSrc, XprGpuStateType blendFactorDest)
{
	if(nullptr == self)
		return;

	self->desc.impl->blendFactorSrcAlpha = blendFactorSrc;
	self->desc.impl->blendFactorDestAlpha = blendFactorDest;
}

void xprGpuStateSetPolygonMode(XprGpuState* self, XprGpuStateType polygonMode)
{
	if(nullptr == self)
		return;

	self->desc.impl->polygonMode = polygonMode;
}
*/