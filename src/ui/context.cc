#include "context.hh"

#include "../input/input_system.hh"
#include "../vk/context.hh"
#include "../win/window.hh"

#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/imgui.h>

namespace vkb::ui
{
	context::context(window& win, input_system& is, vk::context& vk)
	: win_ {win}
	, is_ {is}
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

		if (is_.just_pressed(key::m2))
			demo_ = !demo_;

		if (demo_)
			ImGui::ShowDemoWindow(&demo_);

		refresh += dt;
		++fps;
		if (refresh >= 1.0)
		{
			refresh -= 1.0;
			disp_fps = fps;
			fps = 0;
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
			ImGui::Text("%u (%.3f ms)", disp_fps, io.DeltaTime * 1000.f);
			ImGui::End();
		}

		if (ImGui::Begin("Mouse"))
		{
			if (is_.pressed(key::m1))
				ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "M1");
			else
				ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "M1");

			if (is_.pressed(key::m2))
				ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "M2");
			else
				ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "M2");

			if (is_.pressed(key::m3))
				ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "M3");
			else
				ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "M3");

			if (is_.pressed(key::m4))
				ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "M4");
			else
				ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "M4");

			if (is_.pressed(key::m5))
				ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "M5");
			else
				ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "M5");

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