#ifndef __XPRENDER_BUFFER_D3D9_H__
#define __XPRENDER_BUFFER_D3D9_H__

#include "API.d3d9.h"
#include "Buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprBufferImpl
{
	XprBuffer i;

	IDirect3DVertexBuffer9* d3dvb;
	IDirect3DIndexBuffer9* d3dib;
	
} XprBufferImpl;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_BUFFER_D3D9_H__