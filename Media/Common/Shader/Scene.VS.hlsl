//--------------------------------------------------------------------------------------
// File: Scene.VS.hlsl
//
// The vertex shader file for the BasicHLSL11 sample.  
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
// Input / Output structures
//--------------------------------------------------------------------------------------
struct IA_OUTPUT
{
	float4 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
	uint iInstanceId	: SV_INSTANCEID;
};

struct VS_OUTPUT
{
	float3 vWorldNormal		: WORLDNORMAL;
	float3 vWorldPosition	: WORLDPOSITION;
	float3 vWorldViewDir	: WORLDVIEWDIR;
	float4 vPosition		: SV_POSITION;
	float2 vTexcoord		: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( IA_OUTPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vWorldNormal = mul(Input.vNormal, (float3x3)g_mWorld);
	Output.vWorldPosition = mul(Input.vPosition, g_mWorld).xyz;
	Output.vWorldViewDir = g_vCameraPosition.xyz - Output.vWorldPosition;
	Output.vPosition = mul(float4(Output.vWorldPosition, 1), g_mViewProjection);
	Output.vTexcoord = Input.vTexcoord;
	
	return Output;
}