//--------------------------------------------------------------------------------------
// File: Post.ToneMapping.hlsl
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
Texture2D g_txColor : register( t0 );
Texture2D g_txBloom : register( t1 );

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
#define TONEMAPPING_ON_LUM 1

float4 Main( PS_INPUT Input ) : SV_TARGET
{
	int3 vTexcoord = int3((int2)Input.vPosition.xy, 0);
	
	float4 vOutput = g_txColor.Load(vTexcoord);
	float4 vBloom = g_txBloom.Sample(g_samLinear, Input.vTexcoord);
	
	// tone mapping
	
	float fKey = g_HDRParams.z;
	float fLumMean = g_HDRParams.x + 0.001f;
	
	float fLum = dot(vOutput.xyz, float3(0.27,0.67,0.06));
	
	vOutput.xyz += vBloom.xyz;
	
#if TONEMAPPING_ON_LUM
	fLum = fLum / fKey;
	fLum = fLum / (1 + fLum);
	
	vOutput.xyz *= fLum;
	
#else
	vOutput.xyz = vOutput.xyz / fKey;
	vOutput.xyz = vOutput.xyz / (1 + vOutput.xyz);
	
#endif

	return vOutput;
}

