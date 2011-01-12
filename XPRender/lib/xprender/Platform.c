#include "Platform.h"

#ifndef _MSC_VER
#define _TRUNCATE ((size_t)-1)
#endif

#if defined(XPR_WIN32)

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

void XprDbgStr(const char* str, ...)
{
	char msg[1024] = {0};

    va_list a;
    va_start(a, str);

    _vsnprintf_s(msg, _countof(msg), _TRUNCATE, str, a);
    OutputDebugStringA(msg);
}

#else

#include <stdlib.h>
#include <stdio.h>

void XprDbgStr(const char* str, ...)
{
	char msg[1024] = {0};

    va_list a;
    va_start(a, str);

    _vsnprintf_s(msg, _countof(msg), _TRUNCATE, str, a);
    printf(msg);
}

#endif	// XPR_WIN32