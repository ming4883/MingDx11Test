#include "Platform.h"

#ifndef _MSC_VER
#define _TRUNCATE ((size_t)-1)
#endif

#include <stdlib.h>
#include <stdio.h>

#if defined(XPR_WIN32)

#include <windows.h>

void XprDbgStr(const char* str, ...)
{
	char msg[1024] = {0};

	va_list a;
	va_start(a, str);

	_vsnprintf_s(msg, _countof(msg), _TRUNCATE, str, a);
	OutputDebugStringA(msg);
}

#elif defined(XPR_ANDROID)

#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "xprender", __VA_ARGS__))

void XprDbgStr(const char* str, ...)
{
	/*
	va_list a;
	va_start(a, str);

	char msg[1024] = {0};
	vsnprintf(msg, countof(msg), str, a);
	__android_log_print(ANDROID_LOG_WARN, "pez", msg);
	*/
	LOGI(str);
}

#else

void XprDbgStr(const char* str, ...)
{
	char msg[1024] = {0};

	va_list a;
	va_start(a, str);

	_vsnprintf_s(msg, _countof(msg), _TRUNCATE, str, a);
	printf(msg);
}

#endif	// XPR_WIN32