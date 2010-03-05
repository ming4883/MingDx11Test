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
cbuffer cbPerObject : register( b0 )
{
	matrix g_WorldViewProjection	: packoffset( c0 );
	matrix g_World					: packoffset( c4 );
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
	uint iInstanceId	: SV_INSTANCEID;
};

struct VS_OUTPUT
{
	float3 vNormal		: MYNORMAL;
	float2 vTexcoord	: MYTEXCOORD0;
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	if( Input.iInstanceId == 1 )
		Input.vPosition.xz = Input.vPosition.xz + 2;
	
	Output.vPosition = mul( Input.vPosition, g_WorldViewProjection );
	Output.vNormal = mul( Input.vNormal, (float3x3)g_World );
	Output.vTexcoord = Input.vTexcoord;
	
	return Output;
}

