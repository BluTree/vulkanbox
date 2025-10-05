struct vertex
{
	float4 pos;
};

ParameterBlock<float4x4> static;

struct vertex_out
{
	float4 pos : SV_Position;
	float4 frag_pos;
}

[shader("vertex")]
vertex_out v_main(vertex in)
{
	vertex_out out;

	out.pos = mul(in.pos, static);
	out.frag_pos = in.pos;

	return out;
}

[shader("fragment")]
float4 f_main(vertex_out in)
{
	return in.frag_pos;
}