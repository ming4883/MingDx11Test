//--------------------------------------------------------------------------------------
// File: Post.Blur.hlsl
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
	float4 g_HDRParams				: packoffset(c5);
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
Texture2D g_txSource : register( t0 );

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
#define USE_TEXOBJ_SAMPLE 0

float4 Main( PS_INPUT Input ) : SV_TARGET
{
	// 2d unit poisson disk samplers
	const int iNumSamples = 16;
	float2 vSamples[iNumSamples];
	
	vSamples[ 0] = float2( 0.007937789, 0.73124397);
	vSamples[ 1] = float2(-0.10177308,-0.6509396);
	vSamples[ 2] = float2(-0.9906806,-0.63400936);
	vSamples[ 3] = float2(-0.5583586,-0.3614012);
	vSamples[ 4] = float2( 0.7163085, 0.22836149);
	vSamples[ 5] = float2(-0.65210974, 0.37117887);
	vSamples[ 6] = float2(-0.12714535, 0.112056136);
	vSamples[ 7] = float2( 0.48898065,-0.66669613);
	vSamples[ 8] = float2(-0.9744036, 0.9155904);
	vSamples[ 9] = float2( 0.9274436,-0.9896486);
	vSamples[10] = float2( 0.9782181, 0.90990245);
	vSamples[11] = float2( 0.96427417,-0.25506377);
	vSamples[12] = float2(-0.5021933,-0.9712455);
	vSamples[13] = float2( 0.3091557,-0.17652994);
	vSamples[14] = float2( 0.4665941, 0.96454906);
	vSamples[15] = float2(-0.461774, 0.9360856);
	
#if USE_TEXOBJ_SAMPLE
	/**/
	float fW, fH;
	g_txSource.GetDimensions(fW, fH);
	float2 vBlurRadius = 4 * float2(1 / fW, 1 / fH);
	
	float4 vOutput = 0;
	
	[unroll]
	for(int i=0; i<iNumSamples; ++i)
	{
		vOutput += g_txSource.Sample(g_samLinear, Input.vTexcoord + vSamples[i] * vBlurRadius);
	}
	
	vOutput /= iNumSamples;
	
	return vOutput;
	
#else	// USE_TEXOBJ_SAMPLE
	const float fBlurRadius = 4;
	int3 vTexcoord = int3((int2)Input.vPosition.xy, 0);
	
	float4 vOutput = 0;
	
	[unroll]
	for(int i=0; i<iNumSamples; ++i)
	{
		int3 vTapTexcoord = vTexcoord;
		vTapTexcoord.xy += (int2)(vSamples[i] * fBlurRadius);
		vOutput += g_txSource.Load(vTapTexcoord);
	}
	
	vOutput /= iNumSamples;
	
	return vOutput;
	
#endif	// USE_TEXOBJ_SAMPLE
	
	
}

