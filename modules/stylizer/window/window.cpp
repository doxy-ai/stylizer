#include "window.hpp"

#include "stylizer/api/glfw.hpp"

#ifdef __EMSCRIPTEN__
	void glfwInitHint(int hint, int value) {}
#endif

namespace stylizer {

	bool glfw_initialized = false;
	void maybe_initialize_glfw() {
		if(glfw_initialized) return;

		static auto glfw = glfw::init();
	}

	struct window window::create(api::vec2u size, std::string_view name /* = "Stylizer" */, window_config config /* = {} */) {
		maybe_initialize_glfw();

		glfw::WindowHints{
			.resizable = config.resizable,
			.visible = config.visible,
			.decorated = config.decorated,
			.focused = config.focused,
			.autoIconify = config.auto_iconify,
			.floating = config.floating,
			.maximized = config.maximized,
			.centerCursor = config.center_cursor,
			.transparentFramebuffer = config.transparent_framebuffer,
			.focusOnShow = config.focus_on_show,
			.scaleToMonitor = config.scale_to_monitor,

			.redBits = config.red_bits,
			.greenBits = config.green_bits,
			.blueBits = config.blue_bits,
			.alphaBits = config.alpha_bits,
			.depthBits = config.depth_bits,
			.stencilBits = config.stencil_bits,
			.accumRedBits = config.accum_red_bits,
			.accumGreenBits = config.accum_green_bits,
			.accumBlueBits = config.accum_blue_bits,
			.accumAlphaBits = config.accum_alpha_bits,

			.auxBuffers = config.aux_buffers,
			.samples = config.samples,
			.refreshRate = config.refresh_rate,
			.stereo = config.stereo,
			.srgbCapable = config.srgb_capable,
			.doubleBuffer = config.double_buffer,

			.clientApi = glfw::ClientApi::None_,
		}.apply();

		return {glfw::Window(size.x, size.y, cstring_from_view(name), nullptr, nullptr)};
	}

#ifndef STYLIZER_USE_ABSTRACT_API
	context window::create_context(const api::device::create_config& config /* = {} */) {
		auto partial = context::create_default(config);
		partial.surface = api::glfw::create_surface<api::current_backend::surface>(raw);
		return context::set_c_pointers(partial);
	}
#endif

} // namespace stylizer