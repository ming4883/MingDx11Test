//--------------------------------------------------------------------------------------
// File: Histogram.Update.VS.hlsl
//
// The vertex shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
};

struct VS_OUTPUT
{
	float4 vPosition : SV_POSITION;
	float2 vTexcoord : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT Main( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition = Input.vPosition;
	Output.vTexcoord = Input.vPosition.xy * float2(1,-1) * 0.5 + 0.5;
	
	return Output;
}

