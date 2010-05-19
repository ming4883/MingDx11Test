//--------------------------------------------------------------------------------------
// File: Histogram.Compute2.GS.hlsl
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
//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct GS_INPUT
{
	float4 vPosition : SV_POSITION;
};

struct GS_OUTPUT
{
	float4 vPosition : SV_POSITION;
	float fBinValue : BINVALUE;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
#define HISTOGRAM_SIZE 16 // this must be matched with the texture-array-size of the histogram!

[maxvertexcount(HISTOGRAM_SIZE)]
void Main(point GS_INPUT Input[1], inout PointStream<GS_OUTPUT> OutputStream )
{
	const int iDiv = (int)g_vInputParams.x;
	const float fHistogramMax = g_vInputParams.z;
	const float fHistogramSize = g_vInputParams.w;
	const float fOutputOffset = (1 / fHistogramSize);
	
	float fHistogram[HISTOGRAM_SIZE];

	// initialize
	[unroll]
	for(int i=0; i<HISTOGRAM_SIZE; ++i)
		fHistogram[i] = 0;
		
	// gather
	int iWInput, iHInput;
	g_txInput.GetDimensions(iWInput, iHInput);
	
	iWInput /= iDiv;
	iHInput /= iDiv;

	int iYOffset = Input[0].vPosition.x / iDiv;
	int iXOffset = Input[0].vPosition.x - iYOffset * iDiv;
	
	iXOffset *= iWInput;
	iYOffset *= iHInput;
	
	[loop]
	for(int y = iYOffset; y < iHInput; ++y)
	{
		[loop]
		for(int x = iXOffset; x < iWInput; ++x)
		{
			float4 vInput = g_txInput.Load(int3(x, y, 0));
			float fIntensity = dot(vInput.xyz, float3(0.27,0.67,0.06));

			//fIntensity = 3.25;
			int i = (int)floor(saturate(fIntensity / fHistogramMax) * fHistogramSize + 0.5);
			fHistogram[i] += 1;
		}
	}

	// output
	[unroll]
	for(int i=0; i<HISTOGRAM_SIZE; ++i)
	{
		GS_OUTPUT Output;
		Output.vPosition.x = ((i / fHistogramSize) * 2 - 1) + fOutputOffset;
		Output.vPosition.y = 0;
		Output.vPosition.z = 0;
		Output.vPosition.w = 1;
		Output.fBinValue = fHistogram[i];
		OutputStream.Append(Output);
		OutputStream.RestartStrip();
	}
}
