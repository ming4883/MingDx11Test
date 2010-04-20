//--------------------------------------------------------------------------------------
// File: Histogram.Update2.PS.hlsl
//
// The pixel shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPostCommon : register( b0 )
{
	matrix g_InvViewProjScaleBias	: packoffset(c0);
	float4 g_ZParams				: packoffset(c4);
	float4 g_UserParams				: packoffset(c5);
};

// g_UserParams = MaxInputValue, KeyTarget, BloomThreshold, Histogram Size

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float4 vPosition : SV_POSITION;
	float2 vTexcoord : TEXCOORD0;
};

SamplerState g_samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2DArray g_txHistogram : register( t0 );

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float BinValue(int bin, int numBins)
{
	return ((float)bin / numBins) * g_UserParams.x;
}

#define MAX_HISTOGRAM_SIZE 64

float4 Main( PS_INPUT Input ) : SV_TARGET
{
	int iW, iH, iD;
	g_txHistogram.GetDimensions(iW, iH, iD);
	
	float freq[MAX_HISTOGRAM_SIZE];
	
	// fetch the histogram into shader registers
	float fTotalFreq = 0;
	for(int i=0; i<iD; ++i)
	{
		 freq[i] = g_txHistogram.Load(int4(0, 0, 0, i)).x;
		 fTotalFreq += freq[i];
	}
	
	// fMean
	float fMean = 0;
	
	for(int i=0; i<iD; ++i)
		fMean += freq[i] * BinValue(i, iD);
	
	fMean /= fTotalFreq;
	
	// max value
	int binMax = iD-1;
	while(freq[binMax] <= 0)
	{
		--binMax;
	}
	
	// key value
	float fTargetFreq = fTotalFreq * (1-g_UserParams.y);
	
	int binKey = iD-1;
	
	float fKeyFreq = 0;
	while(fKeyFreq + freq[binKey] <= fTargetFreq)
	{
		fKeyFreq += freq[binKey];
		--binKey;
	}
	
	float a = BinValue(binKey+1, iD);
	float b = BinValue(binKey, iD);

	float aFreq = fKeyFreq;
	float bFreq = fKeyFreq + freq[binKey];
	float t = (fTargetFreq - aFreq) / (bFreq - aFreq);
	float fKey = a + (b-a) * t;
	
	return float4(
		fMean,
		BinValue(binMax, iD),
		lerp(fKey, fMean, 0.8),
		fKey * g_UserParams.z
		);
}

