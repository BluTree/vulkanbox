struct vertex
{
	float4 pos;
};

ParameterBlock<float4x4> static_data;

struct vertex_out
{
	float4 pos : SV_Position;
	float4 frag_pos;
}

float sq_dist(float4 v1, float4 v2)
{
	return ((v2.x-v1.x)*(v2.x-v1.x))+
		((v2.y-v1.y)*(v2.y-v1.y))+
		((v2.z-v1.z)*(v2.z-v1.z))+
		((v2.w-v1.w)*(v2.w-v1.w));
}

float len(float4 v)
{
	return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

float4 norm(float4 v)
{
	float len = len(v);
	float4 res = v;

	res.x = v.x/len;
	res.y = v.y/len;
	res.z = v.z/len;

	return res;
}

[shader("vertex")]
vertex_out v_main(vertex in)
{
	vertex_out out;

	out.pos = mul(in.pos, static_data);
	out.frag_pos = in.pos;

	return out;
}

static float4 star_pos[] = {
	{1.f, 0.f, 0.f, 1.f},
	{1.f, .4f, 0.4f, 1.f},
	{.9f, .5f, 0.2f, 1.f},
	{1.1f, .6f, 0.3f, 1.f},
	{1.f, .4f, 0.f, 1.f},
	{-1.f, 0.f, 0.f, 1.f},
	{0.f, 1.f, 0.f, 1.f},
	{0.f, -1.f, 0.f, 1.f},
	{0.f, 0.f, 1.f, 1.f},
	{0.f, 0.f, -1.f, 1.f},
};

[shader("fragment")]
float4 f_main(vertex_out in)
{
	float4 col = float4(0.f, 0.f, 0.f, 1.f);

	[unroll]
	for(int i = 0; i < sizeof(star_pos) / sizeof(float4); ++i)
	{
		float ratio = clamp(sq_dist(norm(star_pos[i]), norm(in.frag_pos)) * 20000, 0.f, 1.f);
		col += lerp(float4(1.f, 1.f, 1.f, 1.f), float4(0.f, 0.f, 0.f, 1.f), ratio);
	}
	return col;
}