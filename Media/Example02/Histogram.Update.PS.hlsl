//--------------------------------------------------------------------------------------
// File: Histogram.Update.PS.hlsl
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
Texture2D g_txHistogram : register( t0 );

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
	int iW, iH;
	g_txHistogram.GetDimensions(iW, iH);
	
	float freq[MAX_HISTOGRAM_SIZE];
	
	// fetch the histogram into shader registers
	float fTotalFreq = 0;
	for(int i=0; i<iW; ++i)
	{
		 freq[i] = g_txHistogram.Load(int3(i, 0, 0)).x;
		 fTotalFreq += freq[i];
	}
	
	// fMean
	float fMean = 0;
	
	for(int i=0; i<iW; ++i)
		fMean += freq[i] * BinValue(i, iW);
	
	fMean /= fTotalFreq;
	
	// max value
	int binMax = iW-1;
	while(binMax > 0 && freq[binMax] <= 0)
	{
		--binMax;
	}
	
	// key value
	float fTargetFreq = fTotalFreq * (1-g_UserParams.y);
	
	int binKey = iW-1;
	
	float fKeyFreq = 0;
	while(binKey > 0 && fKeyFreq + freq[binKey] <= fTargetFreq)
	{
		fKeyFreq += freq[binKey];
		--binKey;
	}
	
	float a = BinValue(binKey+1, iW);
	float b = BinValue(binKey, iW);

	float aFreq = fKeyFreq;
	float bFreq = fKeyFreq + freq[binKey];
	float t = (fTargetFreq - aFreq) / (bFreq - aFreq);
	float fKey = a + (b-a) * t;
	
	return float4(
		fMean,
		BinValue(binMax, iW),
		lerp(fKey, fMean, 0.8),
		fKey * g_UserParams.z
		);
}

