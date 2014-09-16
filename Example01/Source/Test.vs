struct vs_in
{
	float4 position	: POSITION;
};

struct vs_out
{
	float4 position	: SV_POSITION;
	float4 texcoord : COLOR;
};

vs_out main (vs_in input)
{
	vs_out output;
	
	output.position = input.position;
	output.texcoord.xy = input.position.xy * float2 (0.5,-0.5) + float2 (0.5, 0.5);
	output.texcoord.zw = float2 (0.0, 1.0);
	
	return output;
}
