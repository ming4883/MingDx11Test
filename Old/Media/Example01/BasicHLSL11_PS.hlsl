//--------------------------------------------------------------------------------------
// File: BasicHLSL11_PS.hlsl
//
// The pixel shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbDefault : register( b0 )
{
	float4		g_vObjectColor			: packoffset( c0 );
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D	g_txDiffuse : register( t0 );
SamplerState g_samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	float4 vDiffuse = g_txDiffuse.Sample(g_samLinear, Input.vTexcoord);
	//float4 vDiffuse = float4(1,1,1,1);
	float fLight = saturate(dot(normalize(Input.vNormal), float3(0,1,0)) * 0.5 + 0.5);
	return vDiffuse * g_vObjectColor * fLight;
}

