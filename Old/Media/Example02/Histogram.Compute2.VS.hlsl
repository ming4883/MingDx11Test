//--------------------------------------------------------------------------------------
// File: Histogram.Compute2.VS.hlsl
//
// The shader file for the MingDx11Test.  
// 
// Copyright (c) Chan Ka Ming. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbHistogram : register(b0)
{
	float4	g_vInputParams : packoffset(c0);	// pitch, stepsize, histogram max value, histogram size
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
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
VS_OUTPUT Main(VS_INPUT Input)
{
	const uint iDiv = (uint)g_vInputParams.x;
	
	VS_OUTPUT Output;
	
	Output.vPosition.y = Input.iInstanceId / iDiv;
	Output.vPosition.x = Input.iInstanceId - (Output.vPosition.y * iDiv);
	Output.vPosition.zw = 0;
	
	return Output;
}
