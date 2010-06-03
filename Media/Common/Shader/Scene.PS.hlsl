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
cbuffer cbSceneConstants : register( b0 )
{
	matrix g_mWorld;
	matrix g_mView;
	matrix g_mProjection;
	matrix g_mViewProjection;
	float4 g_vCameraPosition;
	float4 g_vCameraParams;
	float4 g_vAmbientColor;
	float4 g_vLightVector;
	float4 g_vLightColor;
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
	float4 vSpecular = vDiffuse.w;
	float4 vAmbient = vDiffuse * g_vAmbientColor;
	vAmbient.xyz *= g_vAmbientColor.w;
	
	Input.vWorldNormal = normalize(Input.vWorldNormal);
	Input.vWorldViewDir = normalize(Input.vWorldViewDir);
	
	float3 vLightDir = g_vLightVector.xyz;
	float3 vHalfDir = normalize(vLightDir + Input.vWorldViewDir);
	vDiffuse.xyz = vDiffuse.xyz * saturate(dot(Input.vWorldNormal, vLightDir));
	vSpecular.xyz = vSpecular.xyz * pow(saturate(dot(Input.vWorldNormal, vHalfDir)), 32);
	
	return float4(vAmbient.xyz + (vSpecular + vDiffuse).xyz * g_vLightColor.xyz * g_vLightColor.w, 1);
}

