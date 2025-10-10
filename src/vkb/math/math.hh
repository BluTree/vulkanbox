#pragma once

namespace vkb
{
	struct vec4;
}

// TODO this will need to be correctly splitted later to prevent a mess
namespace vkb::math
{

	void   init_random();
	double rand();

	vec4 generate_sphere_point();
}