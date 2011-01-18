#include "GpuState.gl3.h"

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
}

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
}

void XprGpuState_setBool(XprGpuState* self, XprHashCode state, XprBool value)
{
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuStateFlag_Inited)) {
		XprDbgStr("XprGpuState not inited!\n");
	}

	if(state == XprGpuState_DepthTestEnable) {
		self->impl->depthTest = value;
	}
	else if(state == XprGpuState_DepthWriteEnable) {
		self->impl->depthWrite = value;
	}
	else if(state == XprGpuState_CullingEnable) {
		self->impl->culling = value;
	}
}