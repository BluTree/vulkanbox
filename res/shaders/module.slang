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

struct static_data
{
	Texture2D tex;
	SamplerState sampler;
	// float4[8] palette;
};

struct dynamic_data
{
	camera cam;
};

// struct object_data
// {
// 	int16_t col_id;
// };

ParameterBlock<static_data> static_set;
ParameterBlock<dynamic_data> dynamic_set;
// ParameterBlock<object_data> object_set;

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
	float4x4 mvp = mul(mul(model, dynamic_set.cam.view), dynamic_set.cam.proj);
	out.pos = mul(in.pos, mvp);
	out.col = in.col;
	out.uv = in.uv;

	return out;
}

[shader("fragment")]
float4 f_main(vertex_out in) : SV_Target
{
	float4 col = static_set.tex.Sample(static_set.sampler, in.uv*2);

	return col;
}