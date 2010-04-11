//--------------------------------------------------------------------------------------
// File: Histogram.Update.PS.hlsl
//
// The pixel shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbHistogramUpdate : register( b0 )
{
	float4 g_vInputParams	: packoffset(c0);	// white target, bloom threshold
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float4 vPosition : SV_POSITION;
	float2 vTexcoord : TEXCOORD0;
};

SamplerState g_samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D g_txHistogram : register( t0 );

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 Main( PS_INPUT Input ) : SV_TARGET
{
	int iWDepth, iHDepth;
	g_txHistogram.GetDimensions(iWDepth, iHDepth);
	
	// use the depth-value in the screen center as the focus distance
	float fDepthFocus = 0;
	int x = iWDepth/2;
	int y = iHDepth/2;
	
	fDepthFocus = g_txHistogram.Load(int3(x, y, 0)).x;
	float3 fPosFocus = ScreenToWorldPosition(float4(x, y, fDepthFocus, 1)).xyz;
	
	return float4(fPosFocus, fDepthFocus);
}

