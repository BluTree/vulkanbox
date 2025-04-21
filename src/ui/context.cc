#include "context.hh"

#include "../cam/free.hh"
#include "../input/input_system.hh"
#include "../vk/context.hh"
#include "../win/window.hh"

#include "../math/quat.hh"
#include "../math/trig.hh"

#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/imgui.h>

namespace vkb::ui
{
	context::context(window& win, input_system& is, vk::context& vk, cam::free& cam)
	: win_ {win}
	, is_ {is}
	, vk_ {vk}
	, cam_ {cam}
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

		if (is_.just_pressed(key::backquote))
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

		// TODO Better debug ui
		/*if (ImGui::Begin("Mouse"))
		{
		    if (is_.pressed(key::w))
		        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "W");
		    else
		        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "W");

		    if (is_.pressed(key::s))
		        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "S");
		    else
		        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "S");

		    if (is_.pressed(key::a))
		        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "A");
		    else
		        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "A");

		    if (is_.pressed(key::d))
		        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "D");
		    else
		        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "D");

		    if (is_.pressed(key::l_shift))
		        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "LSHIFT");
		    else
		        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "LSHIFT");

		    if (is_.pressed(key::l_ctrl))
		        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "LCTRL");
		    else
		        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "LCTRL");

		    ImGui::End();
		}

		if (ImGui::Begin("Cam"))
		{
		    ImGui::Text("Yaw: %.2f (rad: %.2f)", cam_.yaw_, rad(cam_.yaw_));
		    ImGui::Text("Pitch: %.2f (rad: %.2f)", cam_.pitch_, rad(cam_.pitch_));
		    ImGui::Text("Pos: %.2f %.2f %.2f", cam_.pos_.x, cam_.pos_.y, cam_.pos_.z);

		    vec4 look = cam_.rot_.rotate(vec4(0.f, 1.f, 0.f, 1.f));
		    ImGui::Text("Look: %.2f %.2f %.2f", look.x, look.y, look.z);
		    ImGui::End();
		}

		if (ImGui::Begin("Window"))
		{
		    auto [pos_x, pos_y] = win_.position();

		    ImGui::Text("Pos: %i %i", pos_x, pos_y);

		    ImGui::End();
		}*/
	}

	void context::draw()
	{
		ImGui::Render();

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
		                                vk_.current_command_buffer());
	}
}