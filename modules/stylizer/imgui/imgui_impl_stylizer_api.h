// dear imgui: Renderer for WebGPU
// This needs to be used along with a Platform Binding (e.g. GLFW, SDL2, SDL3)
// (Please note that WebGPU is a recent API, may not be supported by all browser, and its ecosystem is generally a mess)

// When targeting native platforms:
//  - One of IMGUI_IMPL_WEBGPU_BACKEND_DAWN or IMGUI_IMPL_WEBGPU_BACKEND_Stylizer *must* be provided.
// When targeting Emscripten:
//  - We now defaults to IMGUI_IMPL_WEBGPU_BACKEND_DAWN is Emscripten version is 4.0.10+, which correspond to using Emscripten '--use-port=emdawnwebgpu'.
//  - We can still define IMGUI_IMPL_WEBGPU_BACKEND_Stylizer to use Emscripten '-s USE_WEBGPU=1' which is marked as obsolete by Emscripten.
// Add #define to your imconfig.h file, or as a compilation flag in your build system.
// This requirement may be removed once WebGPU stabilizes and backends converge on a unified interface.
//#define IMGUI_IMPL_WEBGPU_BACKEND_DAWN
//#define IMGUI_IMPL_WEBGPU_BACKEND_Stylizer

// Implemented features:
//  [X] Renderer: User texture binding. Use 'StylizerTextureView' as ImTextureID. Read the FAQ about ImTextureID/ImTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (ImGuiBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(ImGui_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
//  [X] Renderer: Texture updates support for dynamic font system (ImGuiBackendFlags_RendererHasTextures).

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#pragma once
#include "imgui.h"          // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE

#include <api.hpp>

// Initialization data, for ImGui_ImplStylizerAPI_Init()
struct ImGui_ImplStylizerAPI_InitInfo {
	stylizer::api::device*   Device = nullptr;
	// int                     NumFramesInFlight = 3;
	stylizer::api::texture::format RenderTargetFormat = stylizer::api::texture_format::Undefined;
	stylizer::api::texture::format DepthStencilFormat = stylizer::api::texture_format::Undefined;
	// stylizer::api::render::pipeline::config::sampling PipelineMultisampleState = {}; // TODO: We should definitely allow this to be controlled!
};

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
IMGUI_IMPL_API bool ImGui_ImplStylizerAPI_Init(const ImGui_ImplStylizerAPI_InitInfo& init_info);
IMGUI_IMPL_API void ImGui_ImplStylizerAPI_Shutdown();
IMGUI_IMPL_API void ImGui_ImplStylizerAPI_NewFrame();
IMGUI_IMPL_API bool ImGui_ImplStylizerAPI_RenderDrawData(ImDrawData* draw_data, stylizer::api::render::pass& pass_encoder);

// Use if you want to reset your rendering device without losing Dear ImGui state.
IMGUI_IMPL_API bool ImGui_ImplStylizerAPI_CreateDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplStylizerAPI_InvalidateDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting ImDrawData::Textures = NULL to handle this manually.
IMGUI_IMPL_API void ImGui_ImplStylizerAPI_UpdateTexture(ImTextureData* tex);

#endif // #ifndef IMGUI_DISABLE