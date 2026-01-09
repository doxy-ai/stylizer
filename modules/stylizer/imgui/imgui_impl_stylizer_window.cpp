#include "imgui_impl_stylizer_window.h"

#ifdef STYLIZER_SDL3_AVAILABLE
	#include <stylizer/sdl/window.hpp>
	#include <stylizer/sdl/event.hpp>
	#include <backends/imgui_impl_sdl3.h>
#endif

static thread_local size_t selected_implementation_magic_number = -1;

IMGUI_IMPL_API bool ImGui_ImplStylizerWindow_Init(stylizer::window& init_window) {
#ifdef STYLIZER_SDL3_AVAILABLE
	if(((stylizer::sdl::window*)&init_window)->type == stylizer::sdl::magic_number) {
		auto& window = *(stylizer::sdl::window*)&init_window;
		selected_implementation_magic_number = stylizer::sdl::magic_number;

		return ImGui_ImplSDL3_InitForOther(window.sdl);
	}
#endif
	throw stylizer::api::error("No window backend supported by the ImGUI implementation available!");
}
IMGUI_IMPL_API void ImGui_ImplStylizerWindow_Shutdown() {
	switch(selected_implementation_magic_number) {
#ifdef STYLIZER_SDL3_AVAILABLE
	case stylizer::sdl::magic_number:
		ImGui_ImplSDL3_Shutdown();
		return;
#endif
	default: throw stylizer::api::error("No window backend supported by the ImGUI implementation available!");
	}
}
IMGUI_IMPL_API void ImGui_ImplStylizerWindow_NewFrame() {
	switch(selected_implementation_magic_number) {
#ifdef STYLIZER_SDL3_AVAILABLE
	case stylizer::sdl::magic_number:
		ImGui_ImplSDL3_NewFrame();
		return;
#endif
	default: throw stylizer::api::error("No window backend supported by the ImGUI implementation available!");
	}
}
IMGUI_IMPL_API stylizer::connection_raw ImGui_ImplStylizerWindow_RegisterEventListener(stylizer::context& ctx) {
	switch(selected_implementation_magic_number) {
#ifdef STYLIZER_SDL3_AVAILABLE
	case stylizer::sdl::magic_number:
		return ctx.handle_event.connect([](const stylizer::context::event& e){
			auto event = stylizer::sdl::event2sdl(e);
			if(!event) return;

			ImGui_ImplSDL3_ProcessEvent(&event->sdl);
		});
#endif
	default: throw stylizer::api::error("No window backend supported by the ImGUI implementation available!");
	}
}