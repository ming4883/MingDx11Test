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

XprNvpParser* xprNvpParserAlloc();

void xprNvpParserFree(XprNvpParser* self);

void xprNvpParserInit(XprNvpParser* self, const char* str);

XprBool xprNvpParserNext(XprNvpParser* self, const char** name, const char** value);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_NVPPARSER_H__
