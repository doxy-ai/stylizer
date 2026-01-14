#pragma once

#include <imgui.h>

static constexpr auto ImGuiWindowFlags_Invisible = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar| ImGuiWindowFlags_NoSavedSettings;

namespace stylizer {

	inline static ImFont* SetImGuiStyle_Dark(float main_scale = 1, bool initialize_dark = true) {
		if(initialize_dark)
			ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// ----- Core VS Dark Colors -----
		colors[ImGuiCol_Text]                  = ImVec4(0.82f, 0.82f, 0.82f, 1.00f); // Light gray text
		colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

		colors[ImGuiCol_WindowBg]              = ImVec4(0.15f, 0.15f, 0.16f, 1.00f); // #262626
		colors[ImGuiCol_ChildBg]               = colors[ImGuiCol_WindowBg];
		colors[ImGuiCol_PopupBg]               = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);

		colors[ImGuiCol_Border]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
		colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.27f, 0.27f, 0.28f, 1.00f);
		colors[ImGuiCol_FrameBgActive]         = ImVec4(0.32f, 0.32f, 0.33f, 1.00f);

		colors[ImGuiCol_TitleBg]               = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
		colors[ImGuiCol_TitleBgActive]         = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);

		// VS-style blue (#007ACC)
		ImVec4 blue                            = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);

		colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
		colors[ImGuiCol_ButtonHovered]         = ImVec4(0.27f, 0.27f, 0.28f, 1.00f);
		colors[ImGuiCol_ButtonActive]          = blue;

		colors[ImGuiCol_Header]                = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
		colors[ImGuiCol_HeaderHovered]         = ImVec4(0.27f, 0.27f, 0.28f, 1.00f);
		colors[ImGuiCol_HeaderActive]          = blue;

		colors[ImGuiCol_Separator]             = ImVec4(0.28f, 0.28f, 0.29f, 1.00f);
		colors[ImGuiCol_SeparatorHovered]      = blue;
		colors[ImGuiCol_SeparatorActive]       = blue;

		colors[ImGuiCol_ResizeGrip]            = ImVec4(0.38f, 0.38f, 0.39f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered]     = blue;
		colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.00f, 0.42f, 0.70f, 1.00f);

		colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.35f, 0.35f, 0.36f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.40f, 0.40f, 0.41f, 1.00f);

		colors[ImGuiCol_CheckMark]             = blue;
		colors[ImGuiCol_SliderGrab]            = blue;
		colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.00f, 0.42f, 0.70f, 1.00f);

		colors[ImGuiCol_Tab]                   = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
		colors[ImGuiCol_TabHovered]            = ImVec4(0.27f, 0.27f, 0.28f, 1.00f);
		colors[ImGuiCol_TabActive]             = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
		colors[ImGuiCol_TabUnfocused]          = colors[ImGuiCol_Tab];
		colors[ImGuiCol_TabUnfocusedActive]    = colors[ImGuiCol_TabActive];

		// ----- Spacing + Rounding (VS-like) -----
		style.WindowRounding = 4;
		style.FrameRounding  = 3;
		style.PopupRounding  = 4;
		style.ScrollbarRounding = 6;
		style.GrabRounding   = 3;
		style.TabRounding    = 4;

		style.FramePadding   = ImVec2(8, 6);
		style.ItemSpacing    = ImVec2(10, 8);
		style.IndentSpacing  = 20;
		style.ScrollbarSize  = 16;

		// VS has clean borders
		style.WindowBorderSize = 1.0f;
		style.FrameBorderSize  = 1.0f;
		style.TabBorderSize    = 1.0f;

		return ImGui::GetIO().Fonts->AddFontFromFileTTF("../../font/InterVariable.ttf"); // TODO: How can we embed this?
		style.ScaleAllSizes(main_scale);        
		style.FontScaleDpi = main_scale;       
	}

}