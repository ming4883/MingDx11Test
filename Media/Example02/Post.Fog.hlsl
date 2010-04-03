//--------------------------------------------------------------------------------------
// File: Post.Fog.hlsl
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
	float4 g_HDRParams				: packoffset(c5);
};


//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D g_txColor : register( t0 );
Texture2D g_txDepth : register( t1 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float4 vPosition : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 Main( PS_INPUT Input ) : SV_TARGET
{
	int3 texcoord = int3((int2)Input.vPosition.xy, 0);
	
	float4 vColor = g_txColor.Load(texcoord);
	float  fDepth = g_txDepth.Load(texcoord).r;
	
	// http://www.humus.name/index.php?page=Comments&ID=256
	float  fDepthLinear = 1 / (fDepth * g_ZParams.x + g_ZParams.y);
	
	float fFog = smoothstep(10, 30, fDepthLinear);
	//fFog = pow(fFog, 4);
	//fFog = 0;
	
	return lerp(vColor, float4(1,1,1,vColor.a), fFog);
}

