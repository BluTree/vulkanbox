struct vertex
{
	float4 pos;
};

ParameterBlock<float4x4> dynamic_data;

struct vertex_out
{
	float4 pos : SV_Position;
	float4 frag_pos;
}

[shader("vertex")]
vertex_out v_main(vertex in)
{
	vertex_out out;

	out.pos = mul(in.pos, dynamic_data);
	out.frag_pos = in.pos;

	return out;
}

struct star
{
	float4 pos;
	float intensity;
}
static const uint32_t star_count = 1000;


ParameterBlock<star[star_count]> static_data;

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

[shader("fragment")]
float4 f_main(vertex_out in)
{
	float4 col = float4(0.f, 0.f, 0.f, 1.f);

	[unroll]
	for(int i = 0; i < star_count; ++i)
	{
		float ratio = clamp(sq_dist(norm(static_data[i].pos), norm(in.frag_pos)) * (20000 / static_data[i].intensity), 0.f, 1.f);
		col += lerp(float4(1.f, 1.f, 1.f, 1.f), float4(0.f, 0.f, 0.f, 1.f), ratio);
	}
	return col;
}