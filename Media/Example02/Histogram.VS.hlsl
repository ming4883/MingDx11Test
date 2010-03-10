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
	float4	g_vInputParams : packoffset(c0);	// pitch, histogram max value
};

Texture2D g_txInput : register(t0);

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
	
	int iInputPitch = (int)g_vInputParams.x;
	float fHistogramMax = g_vInputParams.y;
	
	int3 vTexcoord;
	vTexcoord.y = Input.iInstanceId / iInputPitch;
	vTexcoord.x = Input.iInstanceId - vTexcoord.y * iInputPitch;
	vTexcoord.z = 0;
	
	float4 vInput = g_txInput.Load(vTexcoord);
	float fIntensity = dot(vInput.xyz, float3(0.3333, 0.3334, 0.3333));
	
	Output.vPosition.x = saturate(fIntensity / fHistogramMax) * 2.0 - 1.0;
	Output.vPosition.y = 0;
	Output.vPosition.z = 0;
	Output.vPosition.w = 1;
	
	return Output;
}
