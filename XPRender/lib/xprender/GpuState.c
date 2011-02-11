#include "GpuState.gl.h"

XprGpuState* XprGpuState_alloc()
{
	XprGpuState* self;
	XprAllocWithImpl(self, XprGpuState, XprGpuStateImpl);

	return self;
}

void XprGpuState_free(XprGpuState* self)
{
	if(nullptr == self)
		return;

	free(self);
}

void XprGpuState_init(XprGpuState* self)
{
	self->flags = XprGpuStateFlag_Inited;
	self->impl->depthTest = XprTrue;
	self->impl->depthWrite = XprTrue;
	self->impl->culling = XprTrue;
	self->impl->blending = XprFalse;
	self->impl->polygonMode = XprGpuState_PolygonMode_Fill;
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

void XprGpuState_preRender(XprGpuState* self)
{
	if(self->impl->depthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if(self->impl->depthWrite)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	if(self->impl->culling)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if(self->impl->blending) {
		glEnable(GL_BLEND);
		
		glBlendFuncSeparate(
			XprGpuState_blendFactorMapping[self->impl->blendFactorSrc - XprGpuState_BlendFactor_One],
			XprGpuState_blendFactorMapping[self->impl->blendFactorDest - XprGpuState_BlendFactor_One],
			XprGpuState_blendFactorMapping[self->impl->blendFactorSrcAlpha - XprGpuState_BlendFactor_One],
			XprGpuState_blendFactorMapping[self->impl->blendFactorDestAlpha - XprGpuState_BlendFactor_One]);
	}
	else {
		glDisable(GL_BLEND);
	}

#if !defined(XPR_GLES_2)
	glPolygonMode(GL_FRONT_AND_BACK, XprGpuState_polygonModeMapping[self->impl->polygonMode - XprGpuState_PolygonMode_Line]);
#endif
}

void XprGpuState_setDepthTestEnabled(XprGpuState* self, XprBool value)
{
	if(nullptr == self)
		return;

	self->impl->depthTest = value;
}

void XprGpuState_setDepthWriteEnabled(XprGpuState* self, XprBool value)
{
	if(nullptr == self)
		return;

	self->impl->depthWrite = value;
}

void XprGpuState_setCullEnabled(XprGpuState* self, XprBool value)
{
	if(nullptr == self)
		return;

	self->impl->culling = value;
}

void XprGpuState_setBlendEnabled(XprGpuState* self, XprBool value)
{
	if(nullptr == self)
		return;

	self->impl->blending = value;
}

void XprGpuState_setBlendFactorRGB(XprGpuState* self, XprGpuStateType blendFactorSrc, XprGpuStateType blendFactorDest)
{
	if(nullptr == self)
		return;

	self->impl->blendFactorSrc = blendFactorSrc;
	self->impl->blendFactorDest = blendFactorDest;
}

void XprGpuState_setBlendFactorA(XprGpuState* self, XprGpuStateType blendFactorSrc, XprGpuStateType blendFactorDest)
{
	if(nullptr == self)
		return;

	self->impl->blendFactorSrcAlpha = blendFactorSrc;
	self->impl->blendFactorDestAlpha = blendFactorDest;
}

void XprGpuState_setPolygonMode(XprGpuState* self, XprGpuStateType polygonMode)
{
	if(nullptr == self)
		return;

	self->impl->polygonMode = polygonMode;
}