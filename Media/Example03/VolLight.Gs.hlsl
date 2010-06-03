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
	float4 vViewSphere  : SPHERE;
};

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(6)]
void Main(point VS_OUTPUT Input[1], inout TriangleStream<GS_OUTPUT> OutputStream )
{
	float3 vertex[4] = {
		float3(-1, 1, 0),
		float3(-1,-1, 0),
		float3( 1, 1, 0),
		float3( 1,-1, 0),
	};
	
	int index[6] = {0, 1, 2, 3, 2, 1};
	
	float4 vViewPosition = mul(float4(g_vVolSphere.xyz, 1), g_mView);
	
	GS_OUTPUT Output;
	[unroll]
	for(int i=0; i<6; ++i)
	{
		Output.vPosition = vViewPosition;
		Output.vPosition.xyz += vertex[index[i]] * g_vVolSphere.w;
		Output.vPosition = mul(Output.vPosition, g_mProjection);
		
		Output.vViewSphere = float4(vViewPosition.xyz, g_vVolSphere.w);
		
		OutputStream.Append(Output);
		
		if(i==2 || i==5)
			OutputStream.RestartStrip();
	}
}
