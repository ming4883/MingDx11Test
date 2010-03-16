//--------------------------------------------------------------------------------------
// File: Histogram.Draw.VS.hlsl
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
	float4	g_vDrawParams : packoffset(c0);	// offsetx, offsety, scalex, scaley
};

Texture2D g_txHistogram : register(t0);

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
	VS_OUTPUT Output;
	
	int iInputPitch = (int)g_vDrawParams.x;
	float fHistogramMax = g_vDrawParams.y;
	
	int3 vTexcoord;
	vTexcoord.x = Input.iInstanceId;
	vTexcoord.y = 0;
	vTexcoord.z = 0;
	
	float fHistogram = g_txHistogram.Load(vTexcoord).x;
	
	Output.vPosition.x = g_vDrawParams.x + g_vDrawParams.z * Input.iInstanceId;
	Output.vPosition.y = g_vDrawParams.y + g_vDrawParams.w * fHistogram + 0.01;
	Output.vPosition.z = 0;
	Output.vPosition.w = 1;
	
	return Output;
}
