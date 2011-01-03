#ifndef __XPRENDER_PLATFORM_H__
#define __XPRENDER_PLATFORM_H__

#if defined(_MSC_VER)
#	define XPR_VC

#elif defined(__GNUC__)
#	define XPR_GCC

#endif

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define nullptr 0

typedef char XprBool;
#define XprFalse 0
#define XprTrue 1

typedef int XprHandle;

void XprDbgStr(const char* str, ...);

#define XprCountOf(A) (sizeof(A) / sizeof(A[0]))

#define XprAllocWithImpl(obj, CLASS, CLASSIMPL) \
	obj = (CLASS*)malloc(sizeof(CLASS)+sizeof(CLASSIMPL));\
	memset(obj, 0, sizeof(CLASS)+sizeof(CLASSIMPL));\
	obj->impl = (CLASSIMPL*)((char*)obj + sizeof(CLASS));

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_PLATFORM_H__