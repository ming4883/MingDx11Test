#include "Label.windows.h"

Label* Label_alloc()
{
	Label* self;
	XprAllocWithImpl(self, Label, LabelImpl);
	return self;
}

void Label_free(Label* self)
{
	DeleteObject(self->impl->hbmp);
	DeleteDC(self->impl->hdc);
	free(self);
}

void Label_init(Label* self, size_t width, size_t height)
{
	self->impl->hdc = CreateCompatibleDC(nullptr);
	self->impl->hbmp = CreateCompatibleBitmap(self->impl->hdc, width, height);
	SelectObject(self->impl->hdc, self->impl->hbmp);
}
