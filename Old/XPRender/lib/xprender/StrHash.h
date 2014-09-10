#ifndef __XPRENDER_STRHASH_H__
#define __XPRENDER_STRHASH_H__

#include "Platform.h"
#include "StrHashMacro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t XprHashCode;
XprHashCode XprHashStruct(const void* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_STRHASH_H__
