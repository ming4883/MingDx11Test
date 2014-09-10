//--------------------------------------------------------------------------------------
// File: Histogram.Draw.GS.hlsl
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
	float4	g_vDrawParams : packoffset(c0);	// pitch, histogram max value
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(2)]
void Main(point VS_OUTPUT Input[1], inout LineStream<VS_OUTPUT> OutputStream )
{
	OutputStream.Append(Input[0]);
	
	Input[0].vPosition.y = g_vDrawParams.y;
	
	OutputStream.Append(Input[0]);
	OutputStream.RestartStrip();
}
