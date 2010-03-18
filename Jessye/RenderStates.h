#ifndef RENDERSTATES_H
#define RENDERSTATES_H

#include "Platform.h"

namespace js
{

struct RenderStates
{
	static void init(D3D11_DEPTH_STENCIL_DESC& desc);

	static void init(D3D11_BLEND_DESC& desc);

	static void init(D3D11_RASTERIZER_DESC& desc);

	static void init(D3D11_SAMPLER_DESC& desc);
};	// RenderStates

}	// namespace js

#endif	// RENDERSTATES_H