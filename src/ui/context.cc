#include "context.hh"

#include "../vk/context.hh"
#include "../win/window.hh"

#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/imgui.h>

namespace vkb::ui
{
	context::context(window& win, vk::context& vk)
	: win_ {win}
	, vk_ {vk}
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplWin32_Init(win_.native_handle());

		ImGui_ImplVulkan_InitInfo init_info {};
		vk_.fill_init_info(init_info);
		ImGui_ImplVulkan_Init(&init_info);
	}

	context::~context()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void context::update()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (ImGui::IsKeyPressed(ImGuiKey_Space))
			demo_ = !demo_;

		if (demo_)
			ImGui::ShowDemoWindow(&demo_);
	}

	void context::draw()
	{
		ImGui::Render();

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
		                                vk_.current_command_buffer());
	}

}