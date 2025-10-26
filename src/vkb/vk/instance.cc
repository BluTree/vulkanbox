#include "instance.hh"

#include "../log.hh"
#include <string.h>
#include <string_view.hh>
#include <vector.hh>

#include "enum_string_helper.hh"
#include "surface.hh"

namespace vkb::vk
{
	namespace
	{
		void callback_print(VkDebugUtilsMessageSeverityFlagBitsEXT message_level,
		                    char const*                            format, ...)
		{
			va_list args;
			va_start(args, format);
			switch (message_level)
			{
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
					log::debug_v(format, args);
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
					log::info_v(format, args);
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
					log::warn_v(format, args);
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
					log::error_v(format, args);
					break;

				default: break;
			}
			va_end(args);
		}
	}

	instance* instance::instance_ {nullptr};

	instance& instance::get()
	{
		return *instance_;
	}

	instance::instance(bool enable_validation)
	{
		instance_ = this;

		bool created = create_instance(enable_validation);
		log::assert(created, "Failed to create VkInstance");

		created = register_debug_callback();
		log::assert(created, "Failed to register debug callback");
	}

	instance::~instance()
	{
		instance_ = nullptr;

		// if (transient_command_pool_)
		// 	vkDestroyCommandPool(device_, transient_command_pool_, nullptr);
		if (command_pool_)
			vkDestroyCommandPool(device_, command_pool_, nullptr);

		if (allocator_)
			vmaDestroyAllocator(allocator_);

		if (device_)
			vkDestroyDevice(device_, nullptr);

		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroy =
			reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
				vkGetInstanceProcAddr(inst_, "vkDestroyDebugUtilsMessengerEXT"));
		if (vkDestroy)
			vkDestroy(inst_, debug_messenger_, nullptr);

		if (inst_)
			vkDestroyInstance(inst_, nullptr);
	}

	void instance::create_device(surface const& surface)
	{
		bool created = select_physical_device(surface);
		log::assert(created, "Failed to find suitable physical device");

		created = create_logical_device();
		log::assert(created, "Failed to create logical device");

		created = create_allocator();
		log::assert(created, "Failed to create allocator");

		created = create_command_pools();
		log::assert(created, "Failed to create command pools");
	}

	VkInstance instance::get_instance()
	{
		return inst_;
	}

	instance::queue_indices instance::get_queue_indices()
	{
		return queue_indices_;
	}

	VkDevice instance::get_device()
	{
		return device_;
	}

	VkPhysicalDevice instance::get_physical_device()
	{
		return phys_device_;
	}

	VmaAllocator instance::get_allocator()
	{
		return allocator_;
	}

	VkQueue instance::get_graphics_queue()
	{
		return graphics_queue_;
	}

	VkQueue instance::get_present_queue()
	{
		return present_queue_;
	}

	VkFormat instance::find_supported_format(mc::array_view<VkFormat> formats,
	                                         VkImageTiling            tiling,
	                                         VkFormatFeatureFlags     feats)
	{
		for (uint32_t i {0}; i < formats.size(); ++i)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(phys_device_, formats[i], &props);
			if (tiling == VK_IMAGE_TILING_LINEAR &&
			    (props.linearTilingFeatures & feats) == feats)
				return formats[i];
			else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
			         (props.optimalTilingFeatures & feats) == feats)
				return formats[i];
		}

		return VK_FORMAT_UNDEFINED;
	}

	image instance::create_image(uint32_t w, uint32_t h, uint32_t mip_lvl,
	                             VkFormat format, VkImageTiling tiling,
	                             VkImageUsageFlags                      usage,
	                             [[maybe_unused]] VkMemoryPropertyFlags props)
	{
		VkImageCreateInfo img_info {};
		img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		img_info.imageType = VK_IMAGE_TYPE_2D;
		img_info.extent.width = w;
		img_info.extent.height = h;
		img_info.extent.depth = 1;
		img_info.mipLevels = 1;
		img_info.arrayLayers = 1;
		img_info.format = format;
		img_info.tiling = tiling;
		img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		img_info.usage = usage;
		img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		img_info.samples = VK_SAMPLE_COUNT_1_BIT;
		img_info.mipLevels = mip_lvl;

		VmaAllocationCreateInfo alloc_info {};

		VkPhysicalDeviceMemoryProperties const* mem_props;
		vmaGetMemoryProperties(allocator_, &mem_props);
		for (uint32_t i {0}; i < mem_props->memoryTypeCount; ++i)
			if ((mem_props->memoryTypes[i].propertyFlags & props) == props)
			{
				alloc_info.memoryTypeBits = 1 << i;
				break;
			}

		image    result {};
		VkResult res = vmaCreateImage(allocator_, &img_info, &alloc_info, &result.image,
		                              &result.memory, nullptr);
		log::assert(res == VK_SUCCESS, "Failed to create image");

		return result;
	}

	VkImageView instance::create_image_view(VkImage img, VkFormat format,
	                                        VkImageAspectFlags flags, uint32_t mip_lvl)
	{
		VkImageViewCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.image = img;
		create_info.format = format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = flags;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;
		create_info.subresourceRange.levelCount = mip_lvl;

		VkImageView img_view {nullptr};
		VkResult    res = vkCreateImageView(device_, &create_info, nullptr, &img_view);
		log::assert(res == VK_SUCCESS, "Failed to create image view (%s)",
		            string_VkResult(res));

		return img_view;
	}

	void instance::transition_image_layout(
		VkCommandBuffer cmd, VkImage img, VkImageLayout old_layout,
		VkImageLayout new_layout, VkAccessFlags src_access_mask,
		VkAccessFlags dst_access_mask, VkPipelineStageFlags src_stage,
		VkPipelineStageFlags dst_stage, VkImageAspectFlags aspect, uint32_t mip_lvl)
	{
		// TODO VK_KHR_synchronization2
		VkImageMemoryBarrier barrier {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = img;
		barrier.subresourceRange.aspectMask = aspect;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mip_lvl;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = src_access_mask;
		barrier.dstAccessMask = dst_access_mask;

		vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1,
		                     &barrier);
	}

	buffer instance::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
	                               VkMemoryPropertyFlags props)
	{
		VkBufferCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.size = size;
		create_info.usage = usage;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo                 alloc_info {};
		VkPhysicalDeviceMemoryProperties const* mem_props;
		vmaGetMemoryProperties(allocator_, &mem_props);
		for (uint32_t i {0}; i < mem_props->memoryTypeCount; ++i)
			if ((mem_props->memoryTypes[i].propertyFlags & props) == props)
			{
				alloc_info.memoryTypeBits = 1 << i;
				break;
			}

		buffer   result {};
		VkResult res = vmaCreateBuffer(allocator_, &create_info, &alloc_info,
		                               &result.buffer, &result.memory, nullptr);
		log::assert(res == VK_SUCCESS, "Failed to create buffer (%s)",
		            string_VkResult(res));
		return result;
	}

	void instance::destroy_buffer(buffer const& buf)
	{
		vmaDestroyBuffer(allocator_, buf.buffer, buf.memory);
	}

	VkCommandBuffer instance::begin_commands(/*bool transient*/)
	{
		VkCommandBuffer cmd {nullptr};

		/*if (transient)
		{
		    VkCommandBufferAllocateInfo cmd_info {};
		    cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		    cmd_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		    cmd_info.commandPool = transient_command_pool_;
		    cmd_info.commandBufferCount = 1;

		    vkAllocateCommandBuffers(device_, &cmd_info, &cmd);
		}
		else*/
		{
			VkCommandBufferAllocateInfo cmd_info {};
			cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd_info.commandPool = command_pool_;
			cmd_info.commandBufferCount = 1;

			vkAllocateCommandBuffers(device_, &cmd_info, &cmd);
		}

		VkCommandBufferBeginInfo begin {};
		begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd, &begin);

		return cmd;
	}

	void instance::end_commands(VkCommandBuffer cmd)
	{
		vkEndCommandBuffer(cmd);

		VkSubmitInfo submit {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;

		VkFenceCreateInfo fence_info {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkFence waitFence;
		vkCreateFence(device_, &fence_info, nullptr, &waitFence);

		vkQueueSubmit(graphics_queue_, 1, &submit, waitFence);

		vkWaitForFences(device_, 1, &waitFence, VK_TRUE, UINT64_MAX);
		vkDestroyFence(device_, waitFence, nullptr);

		vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);
	}

	mc::vector<VkCommandBuffer> instance::allocate_commands(uint32_t count)
	{
		mc::vector<VkCommandBuffer> cmds(count);

		VkCommandBufferAllocateInfo cmd_info {};
		cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_info.commandPool = command_pool_;
		cmd_info.commandBufferCount = count;

		vkAllocateCommandBuffers(device_, &cmd_info, cmds.data());

		return cmds;
	}

	void instance::free_commands(mc::array_view<VkCommandBuffer> cmds)
	{
		vkFreeCommandBuffers(device_, command_pool_, cmds.size(), cmds.data());
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL
	instance::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_level,
	                         VkDebugUtilsMessageTypeFlagsEXT             message_type,
	                         VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
	                         [[maybe_unused]] void*                      ud)
	{
		if (message_level < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			return VK_FALSE;

		char const* msg_type_str {nullptr};
		switch (message_type)
		{
			case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
				msg_type_str = "[general]";
				break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
				msg_type_str = "[validation]";
				break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
				msg_type_str = "[validation]";
				break;

			default: msg_type_str = "[unknown]"; break;
		}

		uint32_t        prefix_indent = 8 + strlen(msg_type_str) + 1;
		mc::string_view line(callback_data->pMessage);
		uint32_t        pos = line.find('\n');
		if (pos != UINT32_MAX)
		{
			callback_print(message_level, "[vulkan]%s %.*s", msg_type_str, pos,
			               line.data());
			line.remove_prefix(pos + 1);
			do
			{
				pos = line.find('\n');
				if (pos != UINT32_MAX)
					callback_print(message_level, "%*s%.*s", prefix_indent, "", pos,
					               line.data());
				else
					callback_print(message_level, "%*s%s", prefix_indent, "",
					               line.data());
			}
			while (pos != UINT32_MAX);
		}
		else
		{
			callback_print(message_level, "[vulkan]%s %s", msg_type_str,
			               callback_data->pMessage);
		}

		return VK_FALSE;
	}

	bool instance::create_instance(bool enable_validation)
	{
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "vulkanbox";
		app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		app_info.pEngineName = "none";
		app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_4;

		VkInstanceCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;

		char const* required_exts[] {"VK_KHR_surface",
#ifdef VKB_WINDOWS
		                             "VK_KHR_win32_surface",
#elif defined(VKB_LINUX)
		                             "VK_KHR_wayland_surface",
#endif
		                             VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
		create_info.ppEnabledExtensionNames = required_exts;
		create_info.enabledExtensionCount = 3;

		if (enable_validation)
		{
			char const* layers[] {"VK_LAYER_KHRONOS_validation"};
			if (!check_validation_layers(layers))
				return false;

			create_info.enabledLayerCount = 1;
			create_info.ppEnabledLayerNames = layers;
		}

		uint32_t ext_cnt {0};
		vkEnumerateInstanceExtensionProperties(nullptr, &ext_cnt, nullptr);
		mc::vector<VkExtensionProperties> exts(ext_cnt);
		vkEnumerateInstanceExtensionProperties(nullptr, &ext_cnt, exts.data());
		log::debug("Available exts: ");
		for (uint32_t i {0}; i < exts.size(); ++i)
			log::debug("    %s - %u", exts[i].extensionName, exts[i].specVersion);
		log::debug(" ");

		VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
		debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_create_info.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug_create_info.pfnUserCallback = instance::debug_callback;
		create_info.pNext = &debug_create_info;

		VkResult res = vkCreateInstance(&create_info, nullptr, &inst_);
		return res == VK_SUCCESS;
	}

	bool instance::register_debug_callback()
	{
		VkDebugUtilsMessengerCreateInfoEXT create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = instance::debug_callback;

		PFN_vkCreateDebugUtilsMessengerEXT vkCreate =
			reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
				vkGetInstanceProcAddr(inst_, "vkCreateDebugUtilsMessengerEXT"));

		if (vkCreate)
		{
			VkResult res = vkCreate(inst_, &create_info, nullptr, &debug_messenger_);
			return res == VK_SUCCESS;
		}
		else
			return false;
	}

	bool instance::check_validation_layers(mc::array_view<char const*> const& layers)
	{
		uint32_t layer_cnt {0};
		vkEnumerateInstanceLayerProperties(&layer_cnt, nullptr);
		mc::vector<VkLayerProperties> avail_layers(layer_cnt);
		vkEnumerateInstanceLayerProperties(&layer_cnt, avail_layers.data());

		for (uint32_t i {0}; i < layers.size(); ++i)
		{
			bool found = false;
			for (uint32_t j {0}; j < avail_layers.size(); ++j)
			{
				if (strcmp(layers[i], avail_layers[j].layerName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
				return false;
		}

		return true;
	}

	bool instance::select_physical_device(surface const& surface)
	{
		uint32_t device_cnt {0};
		vkEnumeratePhysicalDevices(inst_, &device_cnt, nullptr);

		if (!device_cnt)
			return false;

		mc::vector<VkPhysicalDevice> devices(device_cnt);
		vkEnumeratePhysicalDevices(inst_, &device_cnt, devices.data());

		uint32_t                    selected = UINT32_MAX;
		VkPhysicalDeviceProperties2 props {};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		VkPhysicalDeviceDriverProperties props2 {};
		props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
		props.pNext = &props2;
		VkPhysicalDeviceFeatures feats;

		uint32_t                          ext_cnt {0};
		mc::vector<VkExtensionProperties> exts;
		for (uint32_t i {0}; i < devices.size(); ++i)
		{
			vkGetPhysicalDeviceProperties2(devices[i], &props);
			vkGetPhysicalDeviceFeatures(devices[i], &feats);

			vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &ext_cnt, nullptr);
			exts.resize(ext_cnt);
			vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &ext_cnt,
			                                     exts.data());

			bool ext_found = false;
			for (uint32_t j {0}; j < exts.size(); ++j)
			{
				if (strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, exts[j].extensionName) == 0)
				{
					ext_found = true;
					break;
				}
			}

			queue_indices              families = find_queue_indices(devices[i], surface);
			surface::swapchain_support support =
				surface.query_swapchain_support(devices[i]);
			if (ext_found &&
			    props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			    feats.geometryShader && feats.samplerAnisotropy &&
			    families.graphics != UINT32_MAX && families.present != UINT32_MAX &&
			    !support.formats.empty() && !support.present_modes.empty())
			{
				selected = i;
				queue_indices_ = families;
				break;
			}
		}

		if (selected == UINT32_MAX)
			return false;

		phys_device_ = devices[selected];
		log::info("Selected device: %s - %s", props.properties.deviceName,
		          props2.driverInfo);
		return true;
	}

	instance::queue_indices instance::find_queue_indices(VkPhysicalDevice device,
	                                                     surface const&   surface)
	{
		queue_indices res;

		uint32_t queue_family_cnt {0};
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_cnt, nullptr);
		mc::vector<VkQueueFamilyProperties> families(queue_family_cnt);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_cnt,
		                                         families.data());

		for (uint32_t i {0}; i < families.size(); ++i)
		{
			if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				res.graphics = i;
			bool present {false};
			present = surface.check_present_queue(device, i);
			if (present)
				res.present = i;
		}

		return res;
	}

	bool instance::create_logical_device()
	{
		[[maybe_unused]] mc::vector<int>    test {0, 1, 2, 3};
		mc::vector<VkDeviceQueueCreateInfo> queues;
		float                               priority {1.f};
		// Graphics Queue
		{
			VkDeviceQueueCreateInfo queue_create_info {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_indices_.graphics;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &priority;
			queues.emplace_back(queue_create_info);
		}

		// Present Queue
		if (queue_indices_.present != queue_indices_.graphics)
		{
			VkDeviceQueueCreateInfo queue_create_info {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_indices_.present;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &priority;
			queues.emplace_back(queue_create_info);
		}

		VkPhysicalDeviceFeatures feats {};
		feats.samplerAnisotropy = VK_TRUE;

		VkPhysicalDeviceVulkan13Features vulkan13_feats {};
		vulkan13_feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		vulkan13_feats.dynamicRendering = true;

		VkDeviceCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pNext = &vulkan13_feats;
		create_info.queueCreateInfoCount = queues.size();
		create_info.pQueueCreateInfos = queues.data();
		create_info.pEnabledFeatures = &feats;

		char const* required_exts[] {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		create_info.enabledExtensionCount = 1;
		create_info.ppEnabledExtensionNames = required_exts;

		VkResult res = vkCreateDevice(phys_device_, &create_info, nullptr, &device_);

		vkGetDeviceQueue(device_, queue_indices_.graphics, 0, &graphics_queue_);
		vkGetDeviceQueue(device_, queue_indices_.present, 0, &present_queue_);
		return res == VK_SUCCESS;
	}

	bool instance::create_allocator()
	{
		VmaAllocatorCreateInfo create_info {};
		create_info.physicalDevice = phys_device_;
		create_info.device = device_;
		create_info.instance = inst_;
		create_info.vulkanApiVersion = VK_API_VERSION_1_4;

		VkResult res = vmaCreateAllocator(&create_info, &allocator_);

		return res == VK_SUCCESS;
	}

	bool instance::create_command_pools()
	{
		VkCommandPoolCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		create_info.queueFamilyIndex = queue_indices_.graphics;

		VkResult res =
			vkCreateCommandPool(device_, &create_info, nullptr, &command_pool_);
		// create_info.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		// VkResult res2 =
		// 	vkCreateCommandPool(device_, &create_info, nullptr, &transient_command_pool_);

		return res == VK_SUCCESS;
	}
} // namespace vkb::vk