struct vs_in
{
	float4 position	: POSITION;
};

struct vs_out
{
	float4 position	: SV_POSITION;
};

vs_out main( vs_in input )
{
	vs_out output;
	
	output.position = input.position;
	
	return output;
}
