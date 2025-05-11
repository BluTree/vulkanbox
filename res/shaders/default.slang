struct vertex
{
	float4 pos;
	float4 col;
	float2 uv;
};

struct camera
{
	float4x4 view;
	float4x4 proj;
};

struct set_0
{
	camera cam;
	Sampler2D tex;
};

ParameterBlock<set_0> set0;

struct vertex_out
{
	float4 pos : SV_Position;
	float4 col;
	float2 uv;
};

[shader("vertex")]
vertex_out v_main(vertex in, uniform float4x4 model)
{
	vertex_out out;
	float4x4 mvp = mul(mul(model, set0.cam.view), set0.cam.proj);
	out.pos = mul(in.pos, mvp);
	out.col = in.col;
	out.uv = in.uv;

	return out;
}

[shader("fragment")]
float4 f_main(vertex_out in) : SV_Target
{
	float4 col = set0.tex.Sample(in.uv*4) * in.col;

	return col;
}