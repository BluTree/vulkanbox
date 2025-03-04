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

	void context::update(double dt)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (ImGui::IsKeyPressed(ImGuiKey_Space))
			demo_ = !demo_;

		if (demo_)
			ImGui::ShowDemoWindow(&demo_);

		refresh += dt;
		if (refresh >= .2)
		{
			refresh -= .2;
			fps = ImGui::GetIO().Framerate;
		}

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->WorkPos.x + 10,
		                               ImGui::GetMainViewport()->WorkPos.y + 10),
		                        ImGuiCond_Always);
		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
		ImGui::SetNextWindowBgAlpha(0.5f);
		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav;
		if (ImGui::Begin("Overlay", nullptr, flags))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("%.2f (%.3f ms)", fps, io.DeltaTime * 1000.f);
			ImGui::End();
		}
	}

	void context::draw()
	{
		ImGui::Render();

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
		                                vk_.current_command_buffer());
	}

}