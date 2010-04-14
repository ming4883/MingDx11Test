//--------------------------------------------------------------------------------------
// File: Post.BrightPass.hlsl
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
Texture2D g_txSource : register( t0 );
Texture2D g_txHDRParams : register( t1 );

#define NUM_ROWS 4
#define NUM_COLS 4
#define NUM_TAPS NUM_ROWS * NUM_COLS

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 Main( PS_INPUT Input ) : SV_TARGET
{
	int3 vTexcoord = int3((int2)Input.vPosition.xy * int2(NUM_COLS, NUM_ROWS), 0);
	
	float4 vOutput = 0;
	
	float fBloomThresshold = g_txHDRParams.Load(int3(0,0,0)).w;
	
	[unroll]
	for(int row=0; row<NUM_ROWS; ++row)
	{
		[unroll]
		for(int col=0; col<NUM_COLS; ++col)
		{
			int3 vTapTexcoord = vTexcoord;
			vTapTexcoord.xy += int2(col, row);
			vOutput += max(g_txSource.Load(vTapTexcoord) - fBloomThresshold, 0);
		}
	}
	
	vOutput /= NUM_TAPS;
	
	return vOutput;
}

