//--------------------------------------------------------------------------------------
// File: Post.DOF.hlsl
//
// The pixel shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPostCommon : register( b0 )
{
	matrix g_InvViewProjScaleBias	: packoffset(c0);
	float4 g_ZParams				: packoffset(c4);
	float4 g_UserParams				: packoffset(c5);
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
Texture2D g_txDepth : register( t0 );

float LinearDepth(float zbufDepth)
{
	// http://www.humus.name/index.php?page=Comments&ID=256
	return 1 / (zbufDepth * g_ZParams.x + g_ZParams.y);
}

float4 ScreenToWorldPosition(float4 screenPos)
{
	// http://www.humus.name/index.php?page=Comments&ID=256
	float4 wPos = mul(screenPos, g_InvViewProjScaleBias);
	return float4(wPos.xyz / wPos.w, 1);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 Main( PS_INPUT Input ) : SV_TARGET
{
	int iWDepth, iHDepth;
	g_txDepth.GetDimensions(iWDepth, iHDepth);
	
	// use the depth-value in the screen center as the focus distance
	float fDepthFocus = 0;
	int x = iWDepth/2;
	int y = iHDepth/2;
	/*
	fDepthFocus += LinearDepth(g_txDepth.Load(int3(x  , y, 0)).r);
	fDepthFocus += LinearDepth(g_txDepth.Load(int3(x+1, y, 0)).r);
	fDepthFocus += LinearDepth(g_txDepth.Load(int3(x  , y+1, 0)).r);
	fDepthFocus += LinearDepth(g_txDepth.Load(int3(x+1, y+1, 0)).r);
	return fDepthFocus / 4;
	*/
	fDepthFocus = g_txDepth.Load(int3(x, y, 0)).x;
	float3 fPosFocus = ScreenToWorldPosition(float4(x, y, fDepthFocus, 1)).xyz;
	
	return float4(fPosFocus, fDepthFocus);
}

