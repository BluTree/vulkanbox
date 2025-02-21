#include "vk/context.hh"
#include "win/window.hh"

#include "log.hh"

#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/imgui.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	vkb::window      main_window;
	vkb::vk::context ctx(main_window);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplWin32_Init(main_window.native_handle());

	ImGui_ImplVulkan_InitInfo init_info {};
	ctx.fill_init_info(init_info);
	ImGui_ImplVulkan_Init(&init_info);

	bool running {ctx.created()};
	// vkb::log::assert(running, "Failed to initialize Vulkan context");

	while (running)
	{
		main_window.update();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow(nullptr);

		if (!main_window.closed() && !main_window.minimized())
		{
			ImGui::Render();
			ctx.draw();
		}

		if (main_window.closed())
			running = false;
	}

	ctx.wait_completion();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	return 0;
}