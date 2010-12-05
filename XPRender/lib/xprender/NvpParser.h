#ifndef __XPRENDER_NVPPARSER_H__
#define __XPRENDER_NVPPARSER_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprNvpParser
{
	char* mStr;
	char* mPos;
} XprNvpParser;

void XprNvpParser_init(XprNvpParser* self, const char* str);

xprBool XprNvpParser_next(XprNvpParser* self, const char** name, const char** value);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_NVPPARSER_H__
