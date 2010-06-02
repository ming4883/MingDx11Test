//--------------------------------------------------------------------------------------
// File: VolLight.GS.hlsl
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
struct VS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(6)]
void Main(point VS_OUTPUT Input[1], inout TriangleStream<VS_OUTPUT> OutputStream )
{
	OutputStream.Append(Input[0]);
	
	
	OutputStream.Append(Input[0]);
	OutputStream.RestartStrip();
}
