#ifndef __XPRENDER_PLATFORM_H__
#define __XPRENDER_PLATFORM_H__

#if defined(_MSC_VER)
#	define XPR_VC

#elif defined(__GNUC__)
#	define XPR_GCC

#endif

#include <stdlib.h>
#include <string.h>
#include "pstdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define nullptr 0

typedef uint8_t XprBool;
#define XprFalse 0
#define XprTrue 1

void xprDbgStr(const char* str, ...);

#define xprCountOf(A) (sizeof(A) / sizeof(A[0]))

#define xprMin(a, b) (a < b ? a : b)

#define xprMax(a, b) (a > b ? a : b)

#define XprAllocWithImpl(obj, CLASS, CLASSIMPL) \
	obj = (CLASS*)malloc(sizeof(CLASS)+sizeof(CLASSIMPL));\
	memset(obj, 0, sizeof(CLASS)+sizeof(CLASSIMPL));\
	obj->impl = (CLASSIMPL*)((char*)obj + sizeof(CLASS));

typedef size_t (* StreamRead) (void* buff, size_t elsize, size_t nelem, void* handle);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_PLATFORM_H__