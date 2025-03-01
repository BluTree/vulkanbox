#include "context.hh"
#include "../log.hh"
#include "../math/trig.hh"
#include "../win/window.hh"

#include <imgui/backends/imgui_impl_vulkan.h>
#include <win32/misc.h>

#include <vulkan/vulkan_win32.h>

#include <stdio.h>
#include <string.h>

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

	context::context(window const& win)
	: win_ {win}
	{
		triangle_.verts = {
			{{-1.0f, -1.0f, -0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
			{{1.0f, -1.0f, -0.0f, 1.0f},  {0.0f, 0.0f, 1.0f, 1.0f}},
			{{1.0f, 1.0f, -0.0f, 1.0f},   {0.0f, 1.0f, 0.0f, 1.0f}},
			{{-1.0f, 1.0f, -0.0f, 1.0f},  {1.0f, 1.0f, 1.0f, 1.0f}},
			{{-1.0f, -1.0f, 1.0f, 1.0f},  {1.0f, 0.0f, 0.0f, 1.0f}},
			{{1.0f, -1.0f, 1.0f, 1.0f},   {0.0f, 0.0f, 1.0f, 1.0f}},
			{{1.0f, 1.0f, 1.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f}},
			{{-1.0f, 1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f, 1.0f}}
        };
		triangle_.idcs = {0, 1, 2, 0, 2, 3};
		triangle_.model = mat4::rotate({0.f, 0.f, 1.f, 1.f}, rad(45));
		auto [w, h] = win_.size();
		proj_ = mat4::persp_proj(0.1f, 10.f, w / (float)h, rad(70));
		view_ = mat4::look_at({0.f, 2.f, 2.f, 1.f}, vec4(), {0.f, 0.f, 1.f, 1.f});

		created_ = create_instance();
		if (!created_)
		{
			log::error("Failed to create VkInstance");
			return;
		}

		created_ = register_debug_callback();
		if (!created_)
		{
			log::error("Failed to register debug callback");
			return;
		}

		created_ = create_surface();
		if (!created_)
		{
			log::error("Failed to create surface");
			return;
		}

		created_ = select_physical_device();
		if (!created_)
		{
			log::error("Failed to find suitable physical device");
			return;
		}

		created_ = create_logical_device();
		if (!created_)
		{
			log::error("Failed to create logical device");
			return;
		}

		created_ = create_swapchain();
		if (!created_)
		{
			log::error("Failed to create swapchain");
			return;
		}

		created_ = create_image_views();
		if (!created_)
		{
			log::error("Failed to create swapchain image views");
			return;
		}

		created_ = create_render_pass();
		if (!created_)
		{
			log::error("Failed to create render pass");
			return;
		}

		created_ = create_desc_set_layout();
		if (!created_)
		{
			log::error("Failed to create descriptor set layout");
			return;
		}

		created_ = create_graphics_pipeline();
		if (!created_)
		{
			log::error("Failed to create graphics pipeline");
			return;
		}

		created_ = create_framebuffers();
		if (!created_)
		{
			log::error("Failed to create framebuffers");
			return;
		}

		created_ = create_command_pool();
		if (!created_)
		{
			log::error("Failed to create command pool");
			return;
		}

		created_ = create_vertex_buffer();
		if (!created_)
		{
			log::error("Failed to create vertex buffer");
			return;
		}

		created_ = create_index_buffer();
		if (!created_)
		{
			log::error("Failed to create index buffer");
			return;
		}

		created_ = create_uniform_buffers();
		if (!created_)
		{
			log::error("Failed to create uniform buffers");
			return;
		}

		created_ = create_command_buffers();
		if (!created_)
		{
			log::error("Failed to create command buffer");
			return;
		}

		created_ = create_sync_objects();
		if (!created_)
		{
			log::error("Failed to create synchronization objects");
			return;
		}

		created_ = create_descriptor_pool();
		if (!created_)
		{
			log::error("Failed to create descriptor pool");
			return;
		}

		created_ = create_descriptor_sets();
		if (!created_)
		{
			log::error("Failed to create descriptor sets");
			return;
		}
	}

	context::~context()
	{
		vkDeviceWaitIdle(device_);

		if (desc_pool_)
			vkDestroyDescriptorPool(device_, desc_pool_, nullptr);

		for (uint8_t i {0}; i < context::max_frames_in_flight; ++i)
		{
			if (in_flight_fences_[i])
				vkDestroyFence(device_, in_flight_fences_[i], nullptr);
			if (draw_end_semaphores_[i])
				vkDestroySemaphore(device_, draw_end_semaphores_[i], nullptr);
			if (img_avail_semaphores_[i])
				vkDestroySemaphore(device_, img_avail_semaphores_[i], nullptr);
			if (uniform_buffers_[i])
			{
				vkDestroyBuffer(device_, uniform_buffers_[i], nullptr);
				vkFreeMemory(device_, uniform_buffers_memory_[i], nullptr);
			}
		}

		if (index_buffer_memory_)
			vkFreeMemory(device_, index_buffer_memory_, nullptr);

		if (index_buffer_)
			vkDestroyBuffer(device_, index_buffer_, nullptr);

		if (vertex_buffer_memory_)
			vkFreeMemory(device_, vertex_buffer_memory_, nullptr);

		if (vertex_buffer_)
			vkDestroyBuffer(device_, vertex_buffer_, nullptr);

		if (command_pool_)
			vkDestroyCommandPool(device_, command_pool_, nullptr);

		for (uint32_t i {0}; i < framebuffers_.size(); ++i)
			vkDestroyFramebuffer(device_, framebuffers_[i], nullptr);

		if (graphics_pipe_)
			vkDestroyPipeline(device_, graphics_pipe_, nullptr);
		if (pipe_layout_)
			vkDestroyPipelineLayout(device_, pipe_layout_, nullptr);
		if (desc_set_layout_)
			vkDestroyDescriptorSetLayout(device_, desc_set_layout_, nullptr);

		if (render_pass_)
			vkDestroyRenderPass(device_, render_pass_, nullptr);

		for (uint32_t i {0}; i < swapchain_image_views_.size(); ++i)
			vkDestroyImageView(device_, swapchain_image_views_[i], nullptr);

		if (swapchain_)
			vkDestroySwapchainKHR(device_, swapchain_, nullptr);

		if (device_)
			vkDestroyDevice(device_, nullptr);

		if (surface_)
			vkDestroySurfaceKHR(inst_, surface_, nullptr);

		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroy =
			reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
				vkGetInstanceProcAddr(inst_, "vkDestroyDebugUtilsMessengerEXT"));
		if (vkDestroy)
			vkDestroy(inst_, debug_messenger_, nullptr);

		if (inst_)
			vkDestroyInstance(inst_, nullptr);
	}

	bool context::created() const
	{
		return created_;
	}

	void context::begin_draw()
	{
		vkWaitForFences(device_, 1, &in_flight_fences_[cur_frame_], VK_TRUE, UINT64_MAX);

		VkResult res = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX,
		                                     img_avail_semaphores_[cur_frame_],
		                                     VK_NULL_HANDLE, &img_idx_);

		if (res == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swapchain();
			return;
		}

		vkResetFences(device_, 1, &in_flight_fences_[cur_frame_]);

		vkResetCommandBuffer(command_buffers_[cur_frame_], 0);
		VkCommandBufferBeginInfo begin_info {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = nullptr;

		res = vkBeginCommandBuffer(command_buffers_[cur_frame_], &begin_info);
		if (res != VK_SUCCESS)
			return;

		VkRenderPassBeginInfo render_pass_info {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = render_pass_;
		render_pass_info.framebuffer = framebuffers_[img_idx_];
		render_pass_info.renderArea.offset = {0, 0};
		render_pass_info.renderArea.extent = swapchain_extent_;

		VkClearValue clear_col {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues = &clear_col;

		vkCmdBeginRenderPass(command_buffers_[cur_frame_], &render_pass_info,
		                     VK_SUBPASS_CONTENTS_INLINE);
	}

	void context::draw()
	{
		struct
		{
			alignas(16) mat4 model;
			alignas(16) mat4 view;
			alignas(16) mat4 proj;
		} ubo;

		ubo.model = triangle_.model;
		ubo.view = view_;
		ubo.proj = proj_;
		memcpy(uniform_buffers_map_[cur_frame_], &ubo, sizeof(ubo));

		record_command_buffer(command_buffers_[cur_frame_]);
	}

	void context::present()
	{
		vkCmdEndRenderPass(command_buffers_[cur_frame_]);

		vkEndCommandBuffer(command_buffers_[cur_frame_]);
		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore          sem_wait[] {img_avail_semaphores_[cur_frame_]};
		VkPipelineStageFlags stages_wait[] {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = sem_wait;
		submit_info.pWaitDstStageMask = stages_wait;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers_[cur_frame_];

		VkSemaphore sem_signal[] {draw_end_semaphores_[cur_frame_]};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = sem_signal;

		vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_fences_[cur_frame_]);

		VkPresentInfoKHR present_info {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = sem_signal;

		VkSwapchainKHR swapchains[] {swapchain_};
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;
		present_info.pImageIndices = &img_idx_;

		VkResult res = vkQueuePresentKHR(present_queue_, &present_info);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
			recreate_swapchain();

		cur_frame_ = (cur_frame_ + 1) % context::max_frames_in_flight;
	}

	void context::fill_init_info(ImGui_ImplVulkan_InitInfo& init_info)
	{
		init_info.Instance = inst_;
		init_info.PhysicalDevice = phys_device_;
		init_info.Device = device_;
		init_info.Queue = graphics_queue_;
		init_info.PipelineCache = pipe_cache_;
		init_info.DescriptorPool = desc_pool_;
		init_info.RenderPass = render_pass_;
		init_info.Subpass = 0;
		init_info.MinImageCount = context::max_frames_in_flight;
		init_info.ImageCount = context::max_frames_in_flight;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
	}

	VkCommandBuffer context::current_command_buffer()
	{
		return command_buffers_[cur_frame_];
	}

	void context::wait_completion()
	{
		vkDeviceWaitIdle(device_);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL
	context::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_level,
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

	bool context::create_instance()
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

		char const* required_exts[] {"VK_KHR_surface", "VK_KHR_win32_surface",
		                             VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
		create_info.ppEnabledExtensionNames = required_exts;
		create_info.enabledExtensionCount = 3;

		char const* layers[] {"VK_LAYER_KHRONOS_validation"};
		if (!check_validation_layers(layers))
			return false;

		create_info.enabledLayerCount = 1;
		create_info.ppEnabledLayerNames = layers;

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
		debug_create_info.pfnUserCallback = context::debug_callback;
		create_info.pNext = &debug_create_info;

		VkResult res = vkCreateInstance(&create_info, nullptr, &inst_);
		return res == VK_SUCCESS;
	}

	bool context::register_debug_callback()
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
		create_info.pfnUserCallback = context::debug_callback;

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

	bool context::check_validation_layers(mc::array_view<char const*> const& layers)
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

	bool context::create_surface()
	{
		VkWin32SurfaceCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		create_info.hwnd = win_.native_handle();
		create_info.hinstance = GetModuleHandleW(nullptr);

		VkResult res = vkCreateWin32SurfaceKHR(inst_, &create_info, nullptr, &surface_);
		return res == VK_SUCCESS;
	}

	bool context::select_physical_device()
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

			queue_families    families = find_queue_families(devices[i]);
			swapchain_support support = query_swapchain_support(devices[i]);
			if (ext_found &&
			    props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			    feats.geometryShader && families.graphics != UINT32_MAX &&
			    families.present != UINT32_MAX && !support.formats.empty() &&
			    !support.present_modes.empty())
			{
				selected = i;
				queue_families_ = families;
				swapchain_support_ = support;
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

	context::queue_families context::find_queue_families(VkPhysicalDevice device)
	{
		queue_families res;

		uint32_t queue_family_cnt {0};
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_cnt, nullptr);
		mc::vector<VkQueueFamilyProperties> families(queue_family_cnt);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_cnt,
		                                         families.data());

		for (uint32_t i {0}; i < families.size(); ++i)
		{
			if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				res.graphics = i;
			VkBool32 present {false};
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present);
			if (present)
				res.present = i;
		}

		return res;
	}

	context::swapchain_support context::query_swapchain_support(VkPhysicalDevice device)
	{
		swapchain_support res;
		uint32_t          cnt {0};

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &res.caps);

		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &cnt, nullptr);
		if (cnt)
		{
			res.formats.resize(cnt);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &cnt,
			                                     res.formats.data());
		}

		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &cnt, nullptr);
		if (cnt)
		{
			res.present_modes.resize(cnt);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &cnt,
			                                          res.present_modes.data());
		}

		return res;
	}

	bool context::create_logical_device()
	{
		mc::vector<VkDeviceQueueCreateInfo> queues;
		float                               priority {1.f};
		// Graphics Queue
		{
			VkDeviceQueueCreateInfo queue_create_info {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_families_.graphics;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &priority;
			queues.emplace_back(queue_create_info);
		}

		// Present Queue
		{
			VkDeviceQueueCreateInfo queue_create_info {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_families_.present;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &priority;
			queues.emplace_back(queue_create_info);
		}

		VkPhysicalDeviceFeatures feats {};
		VkDeviceCreateInfo       create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = queues.size();
		create_info.pQueueCreateInfos = queues.data();
		create_info.pEnabledFeatures = &feats;

		char const* required_exts[] {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		create_info.enabledExtensionCount = 1;
		create_info.ppEnabledExtensionNames = required_exts;

		VkResult res = vkCreateDevice(phys_device_, &create_info, nullptr, &device_);

		vkGetDeviceQueue(device_, queue_families_.graphics, 0, &graphics_queue_);
		vkGetDeviceQueue(device_, queue_families_.present, 0, &present_queue_);
		return res == VK_SUCCESS;
	}

	VkSurfaceFormatKHR context::choose_swap_format()
	{
		for (uint32_t i {0}; i < swapchain_support_.formats.size(); ++i)
			if (swapchain_support_.formats[i].format == VK_FORMAT_B8G8R8_SRGB &&
			    swapchain_support_.formats[i].colorSpace ==
			        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return swapchain_support_.formats[i];

		return swapchain_support_.formats[0];
	}

	VkPresentModeKHR context::choose_swap_present_mode()
	{
		for (uint32_t i {0}; i < swapchain_support_.present_modes.size(); ++i)
			if (swapchain_support_.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				return swapchain_support_.present_modes[i];

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D context::choose_swap_extent()
	{
		if (swapchain_support_.caps.currentExtent.width != UINT32_MAX &&
		    swapchain_support_.caps.currentExtent.height != UINT32_MAX)
			return swapchain_support_.caps.currentExtent;
		else
		{
			mc::pair size = win_.size();

			VkExtent2D extent {size.first, size.second};

			if (extent.width < swapchain_support_.caps.minImageExtent.width)
				extent.width = swapchain_support_.caps.minImageExtent.width;
			else if (extent.width > swapchain_support_.caps.maxImageExtent.width)
				extent.width = swapchain_support_.caps.maxImageExtent.width;

			if (extent.height < swapchain_support_.caps.minImageExtent.height)
				extent.height = swapchain_support_.caps.minImageExtent.height;
			else if (extent.height > swapchain_support_.caps.maxImageExtent.height)
				extent.height = swapchain_support_.caps.maxImageExtent.height;

			return extent;
		}
	}

	bool context::create_swapchain()
	{
		VkSurfaceFormatKHR format = choose_swap_format();
		VkPresentModeKHR   present_mode = choose_swap_present_mode();
		VkExtent2D         extent = choose_swap_extent();

		uint32_t img_cnt = swapchain_support_.caps.minImageCount + 1;
		if (swapchain_support_.caps.maxImageCount > 0 &&
		    img_cnt > swapchain_support_.caps.maxImageCount)
			img_cnt = swapchain_support_.caps.maxImageCount;

		VkSwapchainCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = surface_;
		create_info.minImageCount = img_cnt;
		create_info.imageFormat = format.format;
		create_info.imageColorSpace = format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t family_indices[] {queue_families_.graphics, queue_families_.present};

		if (queue_families_.graphics != queue_families_.present)
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = family_indices;
		}
		else
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

		create_info.preTransform = swapchain_support_.caps.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = nullptr;

		VkResult res = vkCreateSwapchainKHR(device_, &create_info, nullptr, &swapchain_);
		if (res != VK_SUCCESS)
			return false;

		swapchain_format_ = format;
		swapchain_extent_ = extent;
		vkGetSwapchainImagesKHR(device_, swapchain_, &img_cnt, nullptr);
		swapchain_images_.resize(img_cnt);
		vkGetSwapchainImagesKHR(device_, swapchain_, &img_cnt, swapchain_images_.data());

		return true;
	}

	bool context::create_image_views()
	{
		swapchain_image_views_.resize(swapchain_images_.size());

		VkImageViewCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = swapchain_format_.format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		for (uint32_t i {0}; i < swapchain_images_.size(); ++i)
		{
			create_info.image = swapchain_images_[i];

			VkResult res = vkCreateImageView(device_, &create_info, nullptr,
			                                 &swapchain_image_views_[i]);

			if (res != VK_SUCCESS)
				return false;
		}

		return true;
	}

	bool context::create_render_pass()
	{
		VkAttachmentDescription color_attachment {};
		color_attachment.format = swapchain_format_.format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attach_ref {};
		color_attach_ref.attachment = 0;
		color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription sub_pass {};
		sub_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		sub_pass.colorAttachmentCount = 1;
		sub_pass.pColorAttachments = &color_attach_ref;

		VkSubpassDependency dep {};
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dep.srcAccessMask = 0;
		dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.attachmentCount = 1;
		create_info.pAttachments = &color_attachment;
		create_info.subpassCount = 1;
		create_info.pSubpasses = &sub_pass;
		create_info.dependencyCount = 1;
		create_info.pDependencies = &dep;

		VkResult res = vkCreateRenderPass(device_, &create_info, nullptr, &render_pass_);
		return res == VK_SUCCESS;
	}

	bool context::create_desc_set_layout()
	{
		VkDescriptorSetLayoutBinding ubo_binding {};
		ubo_binding.binding = 0;
		ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_binding.descriptorCount = 1;
		ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = 1;
		create_info.pBindings = &ubo_binding;

		VkResult res = vkCreateDescriptorSetLayout(device_, &create_info, nullptr,
		                                           &desc_set_layout_);
		return res == VK_SUCCESS;
	}

	bool context::create_graphics_pipeline()
	{
		VkShaderModule vert_shader;
		{
			uint8_t* vert_buf = nullptr;
			int64_t  vert_size = 0;
			FILE*    vert = fopen("res/shaders/default.vert.spirv", "rb");
			if (!vert)
			{
				log::error("Failed to open %s", "res/shaders/default.vert.spirv");
				return false;
			}

			fseek(vert, 0, SEEK_END);
			fgetpos(vert, &vert_size);
			fseek(vert, 0, SEEK_SET);
			vert_buf = new uint8_t[vert_size];
			fread(vert_buf, 1, vert_size, vert);
			fclose(vert);

			vert_shader = create_shader(vert_buf, vert_size);
			if (vert_buf)
				delete[] vert_buf;
		}

		VkShaderModule frag_shader;
		{
			uint8_t* frag_buf = nullptr;
			int64_t  frag_size = 0;
			FILE*    frag = fopen("res/shaders/default.frag.spirv", "rb");
			if (!frag)
			{
				log::error("Failed to open %s", "res/shaders/default.frag.spirv");
				return false;
			}

			fseek(frag, 0, SEEK_END);
			fgetpos(frag, &frag_size);
			fseek(frag, 0, SEEK_SET);
			frag_buf = new uint8_t[frag_size];
			fread(frag_buf, 1, frag_size, frag);

			fclose(frag);
			frag_shader = create_shader(frag_buf, frag_size);
			if (frag_buf)
				delete[] frag_buf;
		}

		VkPipelineShaderStageCreateInfo vert_create_info {};
		vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_create_info.module = vert_shader;
		vert_create_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_create_info {};
		frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_create_info.module = frag_shader;
		frag_create_info.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] {vert_create_info,
		                                                 frag_create_info};

		VkVertexInputBindingDescription binding_desc = object::binding_desc();
		mc::array<VkVertexInputAttributeDescription, 2> attribute_descs =
			object::attribute_descs();

		VkPipelineVertexInputStateCreateInfo vert_input_info {};
		vert_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_info.vertexBindingDescriptionCount = 1;
		vert_input_info.pVertexBindingDescriptions = &binding_desc;
		vert_input_info.vertexAttributeDescriptionCount = attribute_descs.size();
		vert_input_info.pVertexAttributeDescriptions = attribute_descs.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly {};
		input_assembly.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkDynamicState dynamic_states[] {VK_DYNAMIC_STATE_VIEWPORT,
		                                 VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamic_state {};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = 2;
		dynamic_state.pDynamicStates = dynamic_states;

		// VkViewport viewport {};
		// viewport.width = swapchain_extent_.width;
		// viewport.height = swapchain_extent_.height;
		// viewport.minDepth = 0.0f;
		// viewport.maxDepth = 1.0f;

		// VkRect2D scissor {};
		// scissor.extent = swapchain_extent_;

		VkPipelineViewportStateCreateInfo viewport_state {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		// viewport_state.pViewports = &viewport;
		viewport_state.scissorCount = 1;
		// viewport_state.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		VkPipelineMultisampleStateCreateInfo msaa {};
		msaa.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		msaa.sampleShadingEnable = VK_FALSE;
		msaa.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		msaa.minSampleShading = 1.f;
		msaa.pSampleMask = nullptr;
		msaa.alphaToCoverageEnable = VK_FALSE;
		msaa.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_attachment {};
		color_attachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_attachment.blendEnable = VK_FALSE;
		color_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blend {};
		color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend.logicOpEnable = VK_FALSE;
		color_blend.logicOp = VK_LOGIC_OP_COPY;
		color_blend.attachmentCount = 1;
		color_blend.pAttachments = &color_attachment;
		color_blend.blendConstants[0] = 0.f;
		color_blend.blendConstants[1] = 0.f;
		color_blend.blendConstants[2] = 0.f;
		color_blend.blendConstants[3] = 0.f;

		VkPipelineLayoutCreateInfo pipe_layout_info {};
		pipe_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipe_layout_info.setLayoutCount = 1;
		pipe_layout_info.pSetLayouts = &desc_set_layout_;
		pipe_layout_info.pushConstantRangeCount = 0;

		if (vkCreatePipelineLayout(device_, &pipe_layout_info, nullptr, &pipe_layout_) !=
		    VK_SUCCESS)
		{
			vkDestroyShaderModule(device_, vert_shader, nullptr);
			vkDestroyShaderModule(device_, frag_shader, nullptr);

			return false;
		}

		VkGraphicsPipelineCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		create_info.stageCount = 2;
		create_info.pStages = shader_stages;
		create_info.pVertexInputState = &vert_input_info;
		create_info.pInputAssemblyState = &input_assembly;
		create_info.pViewportState = &viewport_state;
		create_info.pRasterizationState = &rasterizer;
		create_info.pMultisampleState = &msaa;
		create_info.pColorBlendState = &color_blend;
		create_info.pDynamicState = &dynamic_state;
		create_info.layout = pipe_layout_;
		create_info.renderPass = render_pass_;
		create_info.subpass = 0;

		VkResult res = vkCreateGraphicsPipelines(device_, nullptr, 1, &create_info,
		                                         nullptr, &graphics_pipe_);

		vkDestroyShaderModule(device_, vert_shader, nullptr);
		vkDestroyShaderModule(device_, frag_shader, nullptr);
		return res == VK_SUCCESS;
	}

	VkShaderModule context::create_shader(uint8_t* spirv, uint32_t spirv_size)
	{
		VkShaderModuleCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = spirv_size;
		create_info.pCode = reinterpret_cast<uint32_t*>(spirv);

		VkShaderModule shader;
		if (vkCreateShaderModule(device_, &create_info, nullptr, &shader) == VK_SUCCESS)
			return shader;
		else
			return nullptr;
	}

	bool context::create_framebuffers()
	{
		framebuffers_.resize(swapchain_image_views_.size());

		for (uint32_t i {0}; i < framebuffers_.size(); ++i)
		{
			VkImageView attachments[] {swapchain_image_views_[i]};

			VkFramebufferCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.renderPass = render_pass_;
			create_info.attachmentCount = 1;
			create_info.pAttachments = attachments;
			create_info.width = swapchain_extent_.width;
			create_info.height = swapchain_extent_.height;
			create_info.layers = 1;

			VkResult res =
				vkCreateFramebuffer(device_, &create_info, nullptr, &framebuffers_[i]);
			if (res != VK_SUCCESS)
				return false;
		}
		return true;
	}

	bool context::create_command_pool()
	{
		VkCommandPoolCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		create_info.queueFamilyIndex = queue_families_.graphics;

		VkResult res =
			vkCreateCommandPool(device_, &create_info, nullptr, &command_pool_);
		return res == VK_SUCCESS;
	}

	bool context::create_vertex_buffer()
	{
		uint64_t buf_size {sizeof(object::vert) * triangle_.verts.size()};

		VkBuffer       staging_buf {nullptr};
		VkDeviceMemory staging_buf_memory {nullptr};

		bool res = create_buffer(buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		                         staging_buf, staging_buf_memory);
		if (!res)
			return false;

		void* buff_mem;
		vkMapMemory(device_, staging_buf_memory, 0, buf_size, 0, &buff_mem);
		memcpy(buff_mem, triangle_.verts.data(), buf_size);
		vkUnmapMemory(device_, staging_buf_memory);

		res = create_buffer(
			buf_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer_, vertex_buffer_memory_);

		copy_buffer(staging_buf, vertex_buffer_, buf_size);

		vkDestroyBuffer(device_, staging_buf, nullptr);
		vkFreeMemory(device_, staging_buf_memory, nullptr);

		return res;
	}

	bool context::create_index_buffer()
	{
		uint64_t buf_size {sizeof(uint16_t) * triangle_.idcs.size()};

		VkBuffer       staging_buf {nullptr};
		VkDeviceMemory staging_buf_memory {nullptr};

		bool res = create_buffer(buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		                         staging_buf, staging_buf_memory);
		if (!res)
			return false;

		void* buff_mem;
		vkMapMemory(device_, staging_buf_memory, 0, buf_size, 0, &buff_mem);
		memcpy(buff_mem, triangle_.idcs.data(), buf_size);
		vkUnmapMemory(device_, staging_buf_memory);

		res = create_buffer(
			buf_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer_, index_buffer_memory_);

		copy_buffer(staging_buf, index_buffer_, buf_size);

		vkDestroyBuffer(device_, staging_buf, nullptr);
		vkFreeMemory(device_, staging_buf_memory, nullptr);

		return res;
	}

	bool context::create_uniform_buffers()
	{
		uint64_t buf_size = 3 * sizeof(mat4);

		for (uint32_t i {0}; i < context::max_frames_in_flight; ++i)
		{
			create_buffer(buf_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			              uniform_buffers_[i], uniform_buffers_memory_[i]);

			vkMapMemory(device_, uniform_buffers_memory_[i], 0, buf_size, 0,
			            &uniform_buffers_map_[i]);
		}

		return true;
	}

	bool context::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
	                            VkMemoryPropertyFlags props, VkBuffer& buf,
	                            VkDeviceMemory& buf_mem)
	{
		VkBufferCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.size = size;
		create_info.usage = usage;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult res = vkCreateBuffer(device_, &create_info, nullptr, &buf);
		if (res != VK_SUCCESS)
			return false;

		VkMemoryRequirements mem_req {};
		vkGetBufferMemoryRequirements(device_, buf, &mem_req);
		VkPhysicalDeviceMemoryProperties mem_props {};
		vkGetPhysicalDeviceMemoryProperties(phys_device_, &mem_props);
		uint32_t mem_type_idx {UINT32_MAX};
		for (uint32_t i {0}; i < mem_props.memoryTypeCount; ++i)
		{
			if ((mem_req.memoryTypeBits & (1 << i)) &&
			    ((mem_props.memoryTypes[i].propertyFlags & props) == props))
			{
				mem_type_idx = i;
				break;
			}
		}
		if (mem_type_idx == UINT32_MAX)
			return false;

		VkMemoryAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_req.size;
		alloc_info.memoryTypeIndex = mem_type_idx;

		res = vkAllocateMemory(device_, &alloc_info, nullptr, &buf_mem);
		vkBindBufferMemory(device_, buf, buf_mem, 0);

		return res == VK_SUCCESS;
	}

	void context::copy_buffer(VkBuffer src, VkBuffer dst, uint64_t size)
	{
		VkCommandBufferAllocateInfo cmd_info {};
		cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_info.commandPool = command_pool_;
		cmd_info.commandBufferCount = 1;

		VkCommandBuffer cmd;
		vkAllocateCommandBuffers(device_, &cmd_info, &cmd);

		VkCommandBufferBeginInfo begin {};
		begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd, &begin);

		VkBufferCopy2 region {};
		region.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
		region.size = size;
		VkCopyBufferInfo2 copy {};
		copy.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
		copy.srcBuffer = src;
		copy.dstBuffer = dst;
		copy.regionCount = 1;
		copy.pRegions = &region;

		vkCmdCopyBuffer2(cmd, &copy);
		vkEndCommandBuffer(cmd);

		VkSubmitInfo submit {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;

		vkQueueSubmit(graphics_queue_, 1, &submit, nullptr);
		vkQueueWaitIdle(graphics_queue_);

		vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);
	}

	bool context::create_command_buffers()
	{
		VkCommandBufferAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = command_pool_;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = context::max_frames_in_flight;

		VkResult res = vkAllocateCommandBuffers(device_, &alloc_info, command_buffers_);
		return res == VK_SUCCESS;
	}

	bool context::create_sync_objects()
	{
		VkSemaphoreCreateInfo sem_info {};
		sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint8_t i {0}; i < context::max_frames_in_flight; ++i)
		{
			VkResult res1 =
				vkCreateSemaphore(device_, &sem_info, nullptr, &img_avail_semaphores_[i]);
			VkResult res2 =
				vkCreateSemaphore(device_, &sem_info, nullptr, &draw_end_semaphores_[i]);
			VkResult res3 =
				vkCreateFence(device_, &fence_info, nullptr, &in_flight_fences_[i]);

			if (res1 != VK_SUCCESS && res2 != VK_SUCCESS && res3 != VK_SUCCESS)
				return false;
		}
		return true;
	}

	bool context::create_descriptor_pool()
	{
		VkDescriptorPoolSize pool_sizes[] = {
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		     IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE                       },
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         context::max_frames_in_flight}
        };
		VkDescriptorPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 0;
		for (VkDescriptorPoolSize& pool_size : pool_sizes)
			pool_info.maxSets += pool_size.descriptorCount;
		pool_info.poolSizeCount = 2;
		pool_info.pPoolSizes = pool_sizes;
		VkResult res = vkCreateDescriptorPool(device_, &pool_info, nullptr, &desc_pool_);

		return res == VK_SUCCESS;
	}

	bool context::create_descriptor_sets()
	{
		VkDescriptorSetLayout layouts[context::max_frames_in_flight] {desc_set_layout_,
		                                                              desc_set_layout_};
		VkDescriptorSetAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = desc_pool_;
		alloc_info.descriptorSetCount = context::max_frames_in_flight;
		alloc_info.pSetLayouts = layouts;

		VkResult res = vkAllocateDescriptorSets(device_, &alloc_info, desc_sets_);
		if (res != VK_SUCCESS)
			return false;

		for (uint32_t i {0}; i < context::max_frames_in_flight; ++i)
		{
			VkDescriptorBufferInfo info {};
			info.buffer = uniform_buffers_[i];
			info.offset = 0;
			info.range = 3 * sizeof(mat4);

			VkWriteDescriptorSet write {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = desc_sets_[i];
			write.dstBinding = 0;
			write.dstArrayElement = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &info;

			vkUpdateDescriptorSets(device_, 1, &write, 0, nullptr);
		}

		return true;
	}

	void context::record_command_buffer(VkCommandBuffer cmd)
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipe_);

		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = swapchain_extent_.width;
		viewport.height = swapchain_extent_.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);

		VkRect2D scissor {};
		scissor.offset = {0, 0};
		scissor.extent = swapchain_extent_;
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		VkBuffer     buffs[] {vertex_buffer_};
		VkDeviceSize offsets[] {0};
		vkCmdBindVertexBuffers(cmd, 0, 1, buffs, offsets);
		vkCmdBindIndexBuffer(cmd, index_buffer_, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_, 0, 1,
		                        &desc_sets_[cur_frame_], 0, nullptr);

		vkCmdDrawIndexed(cmd, triangle_.idcs.size(), 1, 0, 0, 0);
	}

	void context::recreate_swapchain()
	{
		vkDeviceWaitIdle(device_);

		for (uint32_t i {0}; i < framebuffers_.size(); ++i)
			vkDestroyFramebuffer(device_, framebuffers_[i], nullptr);
		for (uint32_t i {0}; i < swapchain_image_views_.size(); ++i)
			vkDestroyImageView(device_, swapchain_image_views_[i], nullptr);

		if (swapchain_)
			vkDestroySwapchainKHR(device_, swapchain_, nullptr);

		swapchain_support_ = query_swapchain_support(phys_device_);
		bool res = create_swapchain();
		res &= create_image_views();
		res &= create_framebuffers();

		auto [w, h] = win_.size();
		proj_ = mat4::persp_proj(0.1f, 10.f, w / (float)h, rad(70));
		if (!res)
			log::error("Cannot recreate swapchain");
	}
} // namespace vkb::vk