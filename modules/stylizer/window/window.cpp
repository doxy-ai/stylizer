#include "window.hpp"

#ifdef __EMSCRIPTEN__
	void glfwInitHint(int hint, int value) {}
#endif

STYLIZER_BEGIN_NAMESPACE

bool glfw_initialized = false;
void maybe_initialize_glfw() {
	if(glfw_initialized) return;

	static auto glfw = glfw::init();
}


struct window window::create(vec2u size, std::string_view name /* = "Stylizer" */, window_config config /* = {} */) {
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

		.clientApi = glfw::ClientApi::None,
	}.apply();

	return {glfw::Window(size.x, size.y, cstring_from_view(name), nullptr, nullptr)};
}


// STYLIZER_PREFIXED(window)* STYLIZER_PREFIXED(create_window) (
// 	STYLIZER_PREFIXED(vec2u) size,
// 	const char* title /*= "Stylizer"*/,
// 	STYLIZER_PREFIXED(window_config) config /*= {}*/
// )  {
// 	return new window(window::create(size, title, config));
// }

// void STYLIZER_PREFIXED(release_window) (
// 	STYLIZER_PREFIXED(window)* window
// )  {
// 	delete window;
// }

// bool STYLIZER_PREFIXED(window_should_close)(
// 	STYLIZER_PREFIXED(window)* window_,
// 	bool poll_events /* = true */
// )  {
// 	struct window& window = *static_cast<struct window*>(window_);
// 	return window.should_close(poll_events);
// }

// vec2u STYLIZER_PREFIXED(window_get_dimensions)(
// 	STYLIZER_PREFIXED(window)* window_
// )  {
// 	struct window& window = *static_cast<struct window*>(window_);
// 	return window.get_dimensions();
// }

// WGPUSurface STYLIZER_PREFIXED(window_get_surface)(
// 	STYLIZER_PREFIXED(window)* window_,
// 	WGPUInstance instance
// )  {
// 	struct window& window = *static_cast<struct window*>(window_);
// 	return window.get_surface(instance);
// }

// STYLIZER_OPTIONAL(STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)) STYLIZER_PREFIXED(window_create_default_state)(
// 	STYLIZER_PREFIXED(window)* window_,
// 	bool prefer_low_power /* = false */
// )  {
// 	struct window& window = *static_cast<struct window*>(window_);
// 	return res2opt(result<STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)>(
// 		window.create_default_state(prefer_low_power)
// 	));
// }

// bool STYLIZER_PREFIXED(window_configure_surface)(
// 	STYLIZER_PREFIXED(window)* window_,
// 	STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
// 	STYLIZER_PREFIXED(surface_configuration) config /* = {} */
// )  {
// 	struct window& window = *static_cast<struct window*>(window_);
// 	return res2opt(window.configure_surface(static_cast<wgpu_state&>(*state), config));
// }

// bool STYLIZER_PREFIXED(window_reconfigure_surface_on_resize)(
// 	STYLIZER_PREFIXED(window)* window_,
// 	STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
// 	STYLIZER_PREFIXED(surface_configuration) config /* = {} */
// )  {
// 	struct window& window = *static_cast<struct window*>(window_);
// 	return res2opt(window.reconfigure_surface_on_resize(static_cast<wgpu_state&>(*state), config));
// }

STYLIZER_END_NAMESPACE