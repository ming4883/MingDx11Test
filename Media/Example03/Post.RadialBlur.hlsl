//--------------------------------------------------------------------------------------
// File: Post.RadialBlur.hlsl
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
Texture2D g_txColor : register( t0 );

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 Main( PS_INPUT Input ) : SV_TARGET
{
	int iWDepth, iHDepth;
	g_txColor.GetDimensions(iWDepth, iHDepth);
	
	const int iNumSamples = 32;
	
	float2 vTarget = g_UserParams.xy;
	float2 vUVDir = vTarget - Input.vTexcoord;
	
	float fUVStep = 0.9 / (float)iNumSamples;
	float fWStep = 1 / (float)iNumSamples;
	
	float4 vOutput = 0;
	float fWSum = 0;
	
	const float2 vBlurRadius = float2(1.0 / iWDepth, 1.0 / iHDepth);
	
	for(int i = 0; i < iNumSamples; ++i)
	{
		float2 vTapTexcoord = Input.vTexcoord + (vUVDir * i * fUVStep);
		float fW = (i+1) * fWStep;
		
		vOutput += g_txColor.SampleLevel(g_samLinear, vTapTexcoord, 0) * fW;
		fWSum += fW;
	}
	
	vOutput /= fWSum;
	
	return vOutput;
}

