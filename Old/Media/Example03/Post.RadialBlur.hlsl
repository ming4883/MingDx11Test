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
	const int iNumSamples = 16;
	
	float2 vTarget = g_UserParams.xy;
	float2 vUVDir = vTarget - Input.vTexcoord;
	
	float fUVStep = g_UserParams.z / (float)iNumSamples;
	float fWStep = 1 / (float)iNumSamples;
	
	float4 vOutput = 0;
	float fWSum = 0;
	
	for(int i = 0; i < iNumSamples; ++i)
	{
		float2 vTapTexcoord = Input.vTexcoord + (vUVDir * i * fUVStep);
		float fW = (i+1) * fWStep;
		//fW = pow(fW, 2);
		
		vOutput += g_txColor.SampleLevel(g_samLinear, vTapTexcoord, 0) * fW;
		fWSum += fW;
	}
	
	vOutput /= fWSum;

	//vOutput += g_txColor.SampleLevel(g_samLinear, Input.vTexcoord, 0);
	
	return vOutput;
}

