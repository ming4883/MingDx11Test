#ifndef __EXAMPLE_LABEL_H__
#define __EXAMPLE_LABEL_H__

#include "../lib/xprender/Platform.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprTexture;

typedef struct LabelImpl;

typedef struct Label 
{
	struct XprTexture* texture;
	struct LabelImpl* impl;
} Label;

Label* Label_alloc();

void Label_free(Label* self);

void Label_init(Label* self, size_t width, size_t height);

void Label_setText(Label* self, const char* text);

void Label_setFont(Label* self, const char* font);

void Label_commit(Label* self, const char* font);


#ifdef __cplusplus
extern }
#endif

#endif // __EXAMPLE_LABEL_H__