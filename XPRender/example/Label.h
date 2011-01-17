#ifndef __EXAMPLE_LABEL_H__
#define __EXAMPLE_LABEL_H__

#include "../lib/xprender/Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LabelImpl;

typedef struct Label 
{
	const char* text;
	const char* font;

	struct LabelImpl* impl;
} Label;

Label* Label_alloc();

void Label_free(Label* self);

void Label_init(Label* self, size_t width, size_t height);


#ifdef __cplusplus
extern }
#endif

#endif // __EXAMPLE_LABEL_H__