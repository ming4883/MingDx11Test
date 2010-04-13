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
cbuffer cbHistogramUpdate : register( b0 )
{
	float4 g_vInputParams	: packoffset(c0);	// MaxInputValue, WhiteTarget, BloomThreshold
};

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
	return ((float)bin / numBins) * g_vInputParams.x;
}

#define MAX_HISTOGRAM_SIZE 64

float4 Main( PS_INPUT Input ) : SV_TARGET
{
	int iW, iH;
	g_txHistogram.GetDimensions(iW, iH);
	
	float freq[MAX_HISTOGRAM_SIZE];
	
	// fetch the histogram into shader registers
	float total = 0;
	for(int i=0; i<iW; ++i)
	{
		 freq[i] = g_txHistogram.Load(int3(i, 0, 0)).x;
		 total += freq;
	}
	
	// mean
	float mean = 0;
	
	for(int i=0; i<iW; ++i)
		mean += freq[i] * BinValue(i, iW);
	
	mean /= total;
	
	// max value
	int binMax = iW-1;
	
	while(freq[binMax] <= 0)
		--binMax;
		
	// white target
	float fTargetValue = total * (1-g_vInputParams.y);
	
	int binWhite = iW-1;
	
	float whiteTotal = 0;
	while(whiteTotal + freq[binWhite] <= fTargetValue)
	{
		whiteTotal += freq[binWhite];
		--binWhite;
	}
	
	float a = BinValue(binWhite+1, iW);
	float b = BinValue(binWhite, iW);

	float aFreq = whiteTotal;
	float bFreq = whiteTotal + histogram[keyIdx];
	float t = (fTargetValue - aFreq) / (bFreq - aFreq);
	float fKey = a + (b-a) * t;
	
	return float4(mean, BinValue(binMax, iW), fKey, fKey * g_vInputParams.z);
	
	
}

