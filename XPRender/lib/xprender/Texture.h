#ifndef __XPRENDER_TEXTURE_H__
#define __XPRENDER_TEXTURE_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprTexture
{
	int name;
	size_t flags;
} XprTexture;

	
#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_TEXTURE_H__