//--------------------------------------------------------------------------------------
// File: BasicHLSL11_VS.hlsl
//
// The vertex shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbDefault : register( b0 )
{
	matrix		g_ViewProjection	: packoffset( c0 );
	matrix		g_World				: packoffset( c4 );
};
#define NUM_INSTANCES 4	// this must be matched with Example01.cpp
cbuffer cbInstancing : register( b1 )
{
	matrix		g_InstanceWorld[NUM_INSTANCES];
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
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Input.vPosition = mul( Input.vPosition, g_World );
	Input.vPosition = mul( Input.vPosition, g_InstanceWorld[Input.iInstanceId] );
	
	Output.vPosition = mul( Input.vPosition, g_ViewProjection );
	
	Output.vNormal = mul( Input.vNormal, (float3x3)g_World );
	Output.vTexcoord = Input.vTexcoord;
	
	return Output;
}

