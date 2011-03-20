#ifndef __XPRENDER_MEMORY_H__
#define __XPRENDER_MEMORY_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void*(* XprAllocFunc) (size_t sizeInBytes, const char* id);
typedef void(* XprFreeFunc) (void* ptr, const char* id);

typedef struct XprMemory
{
	XprAllocFunc alloc;
	XprFreeFunc free;
} XprMemory;

XprMemory* xprMemory();

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_MEMORY_H__