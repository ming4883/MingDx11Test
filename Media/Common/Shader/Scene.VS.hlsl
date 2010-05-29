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
cbuffer cbShared : register( b0 )
{
	matrix g_ViewProjection	: packoffset(c0);
	matrix g_World			: packoffset(c4);
	float3 g_CameraPosition : packoffset(c8);
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
	
	Output.vWorldNormal = mul(Input.vNormal, (float3x3)g_World);
	Output.vWorldPosition = mul(Input.vPosition, g_World).xyz;
	Output.vWorldViewDir = g_CameraPosition - Output.vWorldPosition;
	Output.vPosition = mul(float4(Output.vWorldPosition, 1), g_ViewProjection);
	Output.vTexcoord = Input.vTexcoord;
	
	return Output;
}