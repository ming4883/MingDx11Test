//--------------------------------------------------------------------------------------
// File: Scene.PS.hlsl
//
// The pixel shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbShared : register( b0 )
{
	float4 g_vLightColor : packoffset( c0 );
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D	g_txDiffuse : register( t0 );
SamplerState g_samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float3 vWorldNormal		: WORLDNORMAL;
	float3 vWorldPosition	: WORLDPOSITION;
	float3 vWorldViewDir	: WORLDVIEWDIR;
	float4 vPosition		: SV_POSITION;
	float2 vTexcoord		: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( VS_OUTPUT Input ) : SV_TARGET
{
	float4 vDiffuse = g_txDiffuse.Sample(g_samLinear, Input.vTexcoord);
	vDiffuse *= g_vLightColor.w;
	float4 vSpecular = vDiffuse.w;
	
	Input.vWorldNormal = normalize(Input.vWorldNormal);
	Input.vWorldViewDir = normalize(Input.vWorldViewDir);
	
	float3 vLightDir = float3(0,1,0);
	float3 vHalfDir = normalize(vLightDir + Input.vWorldViewDir);
	vDiffuse.xyz = vDiffuse.xyz * saturate(dot(Input.vWorldNormal, vLightDir) * 0.5 + 0.5);
	vSpecular.xyz = vSpecular.xyz * pow(saturate(dot(Input.vWorldNormal, vHalfDir)), 32);
	
	return float4((vSpecular + vDiffuse).xyz * g_vLightColor.xyz, 1);
}

