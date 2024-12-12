#include "window.hpp"

#include "stylizer/api/glfw.hpp"

#ifdef __EMSCRIPTEN__
	void glfwInitHint(int hint, int value) {}
#endif

namespace stylizer {

	struct GLFW {
		static bool initalized;

		GLFW() { glfwInit(); }
		~GLFW() { glfwTerminate(); }

		static void maybe_initalize() {
			if(initalized) return;
			static GLFW glfw{};
		}
	};
	bool GLFW::initalized = false;

	struct window window::create(api::vec2u size, std::string_view name /* = "Stylizer" */, window_config config /* = {} */) {
		GLFW::maybe_initalize();

		glfwWindowHint(GLFW_RESIZABLE, config.resizable);
		glfwWindowHint(GLFW_VISIBLE, config.visible);
		glfwWindowHint(GLFW_DECORATED, config.decorated);
		glfwWindowHint(GLFW_AUTO_ICONIFY, config.auto_iconify);
		glfwWindowHint(GLFW_FLOATING, config.floating);
		glfwWindowHint(GLFW_MAXIMIZED, config.maximized);
		glfwWindowHint(GLFW_CENTER_CURSOR, config.center_cursor);
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, config.transparent_framebuffer);
		glfwWindowHint(GLFW_FOCUS_ON_SHOW, config.focus_on_show);
		glfwWindowHint(GLFW_SCALE_TO_MONITOR, config.scale_to_monitor);

		glfwWindowHint(GLFW_RED_BITS, config.red_bits);
		glfwWindowHint(GLFW_GREEN_BITS, config.green_bits);
		glfwWindowHint(GLFW_BLUE_BITS, config.blue_bits);
		glfwWindowHint(GLFW_ALPHA_BITS, config.alpha_bits);
		glfwWindowHint(GLFW_DEPTH_BITS, config.depth_bits);
		glfwWindowHint(GLFW_STENCIL_BITS, config.stencil_bits);
		glfwWindowHint(GLFW_ACCUM_RED_BITS, config.accum_red_bits);
		glfwWindowHint(GLFW_ACCUM_GREEN_BITS, config.accum_green_bits);
		glfwWindowHint(GLFW_ACCUM_BLUE_BITS, config.accum_blue_bits);
		glfwWindowHint(GLFW_ACCUM_ALPHA_BITS, config.accum_alpha_bits);

		glfwWindowHint(GLFW_AUX_BUFFERS, config.aux_buffers);
		glfwWindowHint(GLFW_SAMPLES, config.samples);
		glfwWindowHint(GLFW_REFRESH_RATE, config.refresh_rate);
		glfwWindowHint(GLFW_STEREO, config.stereo);
		glfwWindowHint(GLFW_SRGB_CAPABLE, config.srgb_capable);
		glfwWindowHint(GLFW_DOUBLEBUFFER, config.double_buffer);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window out{};
		out.window_ = glfwCreateWindow(size.x, size.y, cstring_from_view(name), nullptr, nullptr);
		glfwSetWindowUserPointer(out.window_, &out);
		glfwSetFramebufferSizeCallback(out.window_, +[](GLFWwindow* window_, int width, int height){
			window& window = *(struct window*)glfwGetWindowUserPointer(window_);
			window.resized(window, width, height);
		});
		return out;
	}

	window& window::operator=(window&& o) {
		window_ = std::exchange(o.window_, nullptr);
		resized = std::move(o.resized);
		glfwSetWindowUserPointer(window_, this);
		return *this;
	}
	bool window::should_close(bool process_events /* = true */) const {
		assert(window_);
		if(process_events) glfwPollEvents();
		return glfwWindowShouldClose(window_);
	}
	api::vec2u window::get_dimensions() const {
		int x, y;
		glfwGetFramebufferSize(window_, &x,&y);
		return {static_cast<size_t>(x), static_cast<size_t>(y)};
	}

#ifndef STYLIZER_USE_ABSTRACT_API
	context window::create_context(const api::device::create_config& config /* = {} */) {
		auto partial = context::create_default(config);
		partial.surface = api::glfw::create_surface<api::current_backend::surface>(window_);
		return context::set_c_pointers(partial);
	}
#endif

} // namespace stylizer