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
cbuffer cbPerObject : register( b0 )
{
	float4 g_vObjectColor : packoffset( c0 );
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
	float4 vSpecular = pow(vDiffuse * 0.5 + 0.5, 2);
	
	Input.vWorldNormal = normalize(Input.vWorldNormal);
	Input.vWorldViewDir = normalize(Input.vWorldViewDir);
	
	float3 vLightDir = float3(0,1,0);
	float3 vHalfDir = normalize(vLightDir + Input.vWorldViewDir);
	vDiffuse.xyz = vDiffuse.xyz * saturate(dot(Input.vWorldNormal, vLightDir) * 0.5 + 0.5);
	vSpecular.xyz = vSpecular.xyz * pow(saturate(dot(Input.vWorldNormal, vHalfDir)), 32);
	
	return vSpecular + vDiffuse * g_vObjectColor;
}

