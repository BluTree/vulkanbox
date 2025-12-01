#pragma once

namespace vkb::game
{
	struct module_base
	{
		module_base* up {nullptr};
		module_base* down {nullptr};
		module_base* left {nullptr};
		module_base* right {nullptr};
		module_base* front {nullptr};
		module_base* back {nullptr};
	};
}