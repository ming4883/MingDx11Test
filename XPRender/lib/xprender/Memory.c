#include "Memory.h"

void* xprDefaultAlloc(size_t sizeInBytes, const char* id)
{
	return realloc(nullptr, sizeInBytes);
}

void xprDefaultFree(void* ptr, const char* id)
{
	realloc(ptr, 0);
}

static XprMemory _xprMemory = 
{
	xprDefaultAlloc,
	xprDefaultFree
};

XprMemory* xprMemory()
{
	return &_xprMemory;
}