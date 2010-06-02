//--------------------------------------------------------------------------------------
// File: VolLight.VS.hlsl
//
// The vertex shader file for the MingDx11Test.  
// 
// Copyright (c) Chan Ka Ming. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbSceneConstants : register( b0 )
{
	matrix g_mWorld;
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
	uint iInstanceId	: SV_INSTANCEID;
};

struct VS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT Main( IA_OUTPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition = mul(float4(Input.vPosition, 1), g_mWorld);
	
	return Output;
}