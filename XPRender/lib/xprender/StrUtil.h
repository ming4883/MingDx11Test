#ifndef __XPRENDER_STRUTIL_H__
#define __XPRENDER_STRUTIL_H__

#include "Platform.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(XPR_VC)
#define strcasecmp stricmp
#endif

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_STRUTIL_H__
