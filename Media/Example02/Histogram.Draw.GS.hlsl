//--------------------------------------------------------------------------------------
// File: Histogram.VS.hlsl
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

Texture2D g_txInput : register(t0);

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
	
	float height = g_txInput.Load(Input[0].vPosition.xyz);
	Input[0].vPosition.y *= height;
	
	OutputStream.Append(Input[0]);
}
