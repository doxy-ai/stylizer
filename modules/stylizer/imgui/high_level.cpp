#include "high_level.hpp"

#include "imgui_impl_stylizer_api.h"
#include "imgui_impl_stylizer_window.h"
#define STYLIZER_INTER_FONT_AVAILABLE
#include "imgui_stylizer_style.h"

#include <SDL3/SDL_video.h>

namespace stylizer {

	imgui::imgui(context& ctx, window& window, api::texture::format color_format, api::texture::format depth_format /* = api::texture::format::Undefined */) {
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup stylizer style
		default_font = stylizer::SetImGuiStyle_Dark(window.content_scaling());		

		ImGui_ImplStylizerWindow_Init(window);
		event_listener_handle = ImGui_ImplStylizerWindow_RegisterEventListener(ctx);
		ImGui_ImplStylizerAPI_Init({
			.Device = &ctx,
			.RenderTargetFormat = color_format,
			.DepthStencilFormat = depth_format
		});
	}

	imgui::~imgui() {
		ImGui_ImplStylizerAPI_Shutdown();
		ImGui_ImplStylizerWindow_Shutdown();
		ImGui::DestroyContext();
	}

	void imgui::begin_frame() {
		ImGui_ImplStylizerWindow_NewFrame();
		ImGui_ImplStylizerAPI_NewFrame();
		ImGui::NewFrame();

	}
	void imgui::end_frame() {
		ImGui::Render();
	}

	stylizer::api::render::pass& imgui::draw(stylizer::api::render::pass& render_pass) {
		ImGui_ImplStylizerAPI_RenderDrawData(ImGui::GetDrawData(), render_pass);
		return render_pass;
	}

}