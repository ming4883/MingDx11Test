#ifndef __XPRENDER_PLATFORM_H__
#define __XPRENDER_PLATFORM_H__

#include <stdlib.h>

#if defined(_MSC_VER)
#	define XPR_VC

#elif defined(__GNUC__)
#	define XPR_GCC

#endif

#ifdef __cplusplus
extern "C" {
#endif

#define nullptr 0

typedef char XprBool;
#define XprFalse 0
#define XprTrue 1


#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_PLATFORM_H__