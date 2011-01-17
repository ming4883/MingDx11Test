#include "DeviceContext.gl3.h"

XprDeviceContext* XprDeviceContext_alloc()
{
	XprDeviceContext* self;
	XprAllocWithImpl(self, XprDeviceContext, XprDeviceContextImpl);

	return self;
}

void XprDeviceContext_free(XprDeviceContext* self)
{
	if(nullptr == self)
		return;

	free(self);
}

void XprDeviceContext_init(XprDeviceContext* self)
{
	self->flags = XprDeviceContextFlag_Inited;
}

void XprDeviceContext_preRender(XprDeviceContext* self)
{
}

void XprDeviceContext_setBool(XprDeviceContext* self, XprHashCode state, XprBool value)
{
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprDeviceContextFlag_Inited)) {
		XprDbgStr("XprDeviceContext not inited!\n");
	}
}