//--------------------------------------------------------------------------------------
// File: Histogram.Compute.GS.hlsl
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
[maxvertexcount(16)]
void Main(point VS_OUTPUT Input[1], inout PointStream<VS_OUTPUT> OutputStream )
{
	const int iInputPitch = (int)g_vInputParams.x;
	//const int iStepSize = (int)g_vInputParams.y;
	const int iStepSize = 4;
	const float fHistogramMax = g_vInputParams.z;
	const float fHistogramSize = g_vInputParams.w;
	const float fOutputOffset = (1 / fHistogramSize);
	
	int3 vTexcoord;
	vTexcoord.y = Input[0].vPosition.x / iInputPitch;
	vTexcoord.x = Input[0].vPosition.x - vTexcoord.y * iInputPitch;
	vTexcoord.xy *= iStepSize;
	vTexcoord.z = 0;
	
	[unroll]
	for(int y = 0; y < iStepSize; ++y)
	{
		[unroll]
		for(int x = 0; x < iStepSize; ++x)
		{
			int3 vOffset = int3(x, y, 0);
			float4 vInput = g_txInput.Load(vTexcoord + vOffset);
			float fIntensity = dot(vInput.xyz, float3(0.3333, 0.3334, 0.3333));
			
			//fIntensity = 3.25;
			
			Input[0].vPosition.x = saturate(fIntensity / fHistogramMax) * 2.0 - 1.0 + fOutputOffset;
			Input[0].vPosition.y = 0;
			Input[0].vPosition.z = 0;
			Input[0].vPosition.w = 1;
			
			OutputStream.Append(Input[0]);
		}
	}
}
