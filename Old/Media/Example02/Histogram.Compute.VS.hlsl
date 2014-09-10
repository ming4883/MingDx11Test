//--------------------------------------------------------------------------------------
// File: Histogram.Compute.VS.hlsl
//
// The shader file for the MingDx11Test.  
// 
// Copyright (c) Chan Ka Ming. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------


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
	
	Output.vPosition.x = Input.iInstanceId;
	Output.vPosition.yzw = 0;
	
	return Output;
}
