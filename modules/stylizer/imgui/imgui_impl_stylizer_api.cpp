#include "imgui_impl_stylizer_api.h"

#include <backends/stub/stub.hpp>

#ifdef STYLIZER_API_WEBGPU_AVAILABLE
	#include <backends/webgpu/common.hpp>
	#include <backends/imgui_impl_wgpu.h>
#endif

static thread_local size_t selected_implementation_magic_number = -1;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
IMGUI_IMPL_API bool ImGui_ImplStylizerAPI_Init(const ImGui_ImplStylizerAPI_InitInfo& init_info) {
#ifdef STYLIZER_API_WEBGPU_AVAILABLE
	if(((stylizer::api::stub::device*)init_info.Device)->type == stylizer::api::webgpu::magic_number) {
		auto& device = init_info.Device->as<stylizer::api::webgpu::device>();
		selected_implementation_magic_number = stylizer::api::webgpu::magic_number;

		ImGui_ImplWGPU_InitInfo info;
		info.Device = device.device_;
		info.RenderTargetFormat = stylizer::api::webgpu::to_webgpu(init_info.RenderTargetFormat);
		info.DepthStencilFormat = stylizer::api::webgpu::to_webgpu(init_info.DepthStencilFormat);
		return ImGui_ImplWGPU_Init(&info);
	}
#endif
	throw stylizer::api::error("No backend supported by the ImGUI implementation available!");
}
IMGUI_IMPL_API void ImGui_ImplStylizerAPI_Shutdown() {
	switch(selected_implementation_magic_number) {
#ifdef STYLIZER_API_WEBGPU_AVAILABLE
	case stylizer::api::webgpu::magic_number:
		ImGui_ImplWGPU_Shutdown();
		return;
#endif
	default: throw stylizer::api::error("No backend supported by the ImGUI implementation available!");
	}
}
IMGUI_IMPL_API void ImGui_ImplStylizerAPI_NewFrame() {
	switch(selected_implementation_magic_number) {
#ifdef STYLIZER_API_WEBGPU_AVAILABLE
	case stylizer::api::webgpu::magic_number:
		ImGui_ImplWGPU_NewFrame();
		return;
#endif
	default: throw stylizer::api::error("No backend supported by the ImGUI implementation available!");
	}
}
IMGUI_IMPL_API bool ImGui_ImplStylizerAPI_RenderDrawData(ImDrawData* draw_data, stylizer::api::render::pass& pass_encoder) {
	switch(selected_implementation_magic_number) {
#ifdef STYLIZER_API_WEBGPU_AVAILABLE
	case stylizer::api::webgpu::magic_number: {
		auto& pass = pass_encoder.as<stylizer::api::webgpu::render_pass>();
		pass.render_used = true; // TODO: Will this cause any problems? With executing an empty pass?
		ImGui_ImplWGPU_RenderDrawData(draw_data, pass.pass);
		return true;
	}
#endif
	default: throw stylizer::api::error("No backend supported by the ImGUI implementation available!");
	}
}

// Use if you want to reset your rendering device without losing Dear ImGui state.
IMGUI_IMPL_API bool ImGui_ImplStylizerAPI_CreateDeviceObjects() {
	switch(selected_implementation_magic_number) {
#ifdef STYLIZER_API_WEBGPU_AVAILABLE
	case stylizer::api::webgpu::magic_number:
		return ImGui_ImplWGPU_CreateDeviceObjects();
#endif
	default: throw stylizer::api::error("No backend supported by the ImGUI implementation available!");
	}
}
IMGUI_IMPL_API void ImGui_ImplStylizerAPI_InvalidateDeviceObjects() {
	switch(selected_implementation_magic_number) {
#ifdef STYLIZER_API_WEBGPU_AVAILABLE
	case stylizer::api::webgpu::magic_number:
		return ImGui_ImplWGPU_InvalidateDeviceObjects();
#endif
	default: throw stylizer::api::error("No backend supported by the ImGUI implementation available!");
	}
}

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting ImDrawData::Textures = NULL to handle this manually.
IMGUI_IMPL_API void ImGui_ImplStylizerAPI_UpdateTexture(ImTextureData* tex) {
	switch(selected_implementation_magic_number) {
#ifdef STYLIZER_API_WEBGPU_AVAILABLE
	case stylizer::api::webgpu::magic_number:
		ImGui_ImplWGPU_UpdateTexture(tex);
		return;
#endif
	default: throw stylizer::api::error("No backend supported by the ImGUI implementation available!");
	}
}