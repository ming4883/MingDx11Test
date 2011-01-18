#include "Label.windows.h"

Label* Label_alloc()
{
	Label* self;
	XprAllocWithImpl(self, Label, LabelImpl);
	return self;
}

void Label_free(Label* self)
{
	if(nullptr != self->impl->text) {
		free(self->impl->text);
	}

	DeleteObject(self->impl->hbmp);
	DeleteDC(self->impl->hdc);
	free(self);
}

void Label_init(Label* self, size_t width, size_t height)
{
	self->impl->width = width;
	self->impl->height = height;
	self->impl->hdc = CreateCompatibleDC(nullptr);
	self->impl->hbmp = CreateBitmap(width, height, 1, 32, nullptr);
	SelectObject(self->impl->hdc, self->impl->hbmp);
}

void Label_setText(Label* self, const char* text)
{
	if(nullptr != self->impl->text) {
		free(self->impl->text);
	}

	self->impl->text = strdup(text);
}

void Label_setFont(Label* self, const char* font)
{
}

void Label_commit(Label* self, const char* font)
{
	RECT rect;
	SetRect(&rect, 0, 0, self->impl->width, self->impl->height);
	SetTextColor(self->impl->hdc, RGB(255,0,0));
	SetBkMode(self->impl->hdc, OPAQUE);
    DrawTextA(self->impl->hdc, self->impl->text, -1, &rect, DT_NOCLIP);
}