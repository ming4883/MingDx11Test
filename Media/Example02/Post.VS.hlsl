//--------------------------------------------------------------------------------------
// File: Scene.VS.hlsl
//
// The vertex shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
/*
cbuffer cbPerObject : register( b0 )
{
	matrix		g_mWorldViewProjection	: packoffset( c0 );
	matrix		g_mWorld				: packoffset( c4 );
};
*/

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
};

struct VS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition = Input.vPosition;
	
	return Output;
}

