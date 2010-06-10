//--------------------------------------------------------------------------------------
// File: VolLight.GS.hlsl
//
// The vertex shader file for the MingDx11Test.  
// 
// Copyright (c) Chan Ka Ming. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbSceneConstants : register( b0 )
{
	matrix g_mWorld;
	matrix g_mView;
	matrix g_mProjection;
	matrix g_mViewProjection;
	float4 g_vCameraPosition;
	float4 g_vCameraParams;
	float4 g_vAmbientColor;
	float4 g_vLightVector;
	float4 g_vLightColor;
};

cbuffer cbVolLightConstants : register( b1 )
{
	float4 g_vVolSphere;	// x y z r
	float4 g_vVolColor;
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
};

struct GS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(36)]
void Main(point VS_OUTPUT Input[1], inout TriangleStream<GS_OUTPUT> OutputStream )
{
	/*
	 6 4
	 7 5

	2 0
	3 1
	*/
	
	float3 vertex[8] = {
		float3( 1, 1, 1),
		float3( 1,-1, 1),
		float3(-1, 1, 1),
		float3(-1,-1, 1),
		float3( 1, 1,-1),
		float3( 1,-1,-1),
		float3(-1, 1,-1),
		float3(-1,-1,-1),
	};
	int index[36] = {
		2, 3, 0, 1, 0, 3,
		4, 5, 6, 7, 6, 5,
		5, 1, 3, 7, 5, 3,
		6, 2, 4, 0, 4, 2,
		0, 1, 4, 5, 4, 1,
		6, 7, 2, 3, 2, 7,
	};
	
	float4 vViewPosition = mul(float4(g_vVolSphere.xyz, 1), g_mView);
	
	GS_OUTPUT Output;
	[unroll]
	for(int i=0; i<36; ++i)
	{
		Output.vPosition = vViewPosition;
		Output.vPosition.xyz += vertex[index[i]] * g_vVolSphere.w * 1.0;
		Output.vPosition = mul(Output.vPosition, g_mProjection);
		
		OutputStream.Append(Output);
		
		if((i%3) == 2)
			OutputStream.RestartStrip();
	}
}
