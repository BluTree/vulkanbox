struct vertex
{
	float4 pos;
	uint instance_id : SV_VertexID;
};

struct camera
{
	float4x4 view;
	float4x4 proj;
};

struct dynamic_data
{
	camera cam;
	float2 translate;
};

ParameterBlock<dynamic_data> dynamic_set;

static const float4 lines_col[] =
{
	{1.f, 1.f, 1.f, 1.f},
	{1.f, 0.f, 0.f, 1.f},
	{0.f, 1.f, 0.f, 1.f},
	{0.f, 0.f, 1.f, 1.f}
};

struct vertex_out
{
	float4 pos : SV_Position;
	float4 col;
};

[shader("vertex")]
vertex_out v_main(vertex in)
{
	vertex_out out;

	float4x4 vp = mul(dynamic_set.cam.view, dynamic_set.cam.proj);
	out.pos = mul(in.pos, vp);
	out.pos.x += dynamic_set.translate.x;
	out.pos.y += dynamic_set.translate.y;

	out.col = lines_col[in.instance_id];

	return out;
}

[shader("fragment")]
float4 f_main(vertex_out in) : SV_Target
{
	return in.col;
}