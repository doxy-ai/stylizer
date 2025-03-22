#include "window.hpp"

#include <SDL3/SDL_init.h>
#include <stylizer/hardware/sdl.hpp>
#include "../build/modules/stylizer/hardware/thirdparty/SDL/include-config-debug/build_config/SDL_build_config.h" // TODO: Why isn't this being auto detected?
#include "SDL3/SDL_error.h"
#include "stylizer/hardware/exceptions.hpp"
#include <stylizer/api/sdl3.hpp>

namespace stylizer {

	struct window window::create(uint2 size, std::string_view name /* = "Stylizer" */, create_config config /* = {} */) {
		// GLFW::maybe_initialize();

		// glfwWindowHint(GLFW_RESIZABLE, config.resizable);
		// glfwWindowHint(GLFW_VISIBLE, config.visible);
		// glfwWindowHint(GLFW_DECORATED, config.decorated);
		// glfwWindowHint(GLFW_AUTO_ICONIFY, config.auto_iconify);
		// glfwWindowHint(GLFW_FLOATING, config.floating);
		// glfwWindowHint(GLFW_MAXIMIZED, config.maximized);
		// glfwWindowHint(GLFW_CENTER_CURSOR, config.center_cursor);
		// glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, config.transparent_framebuffer);
		// glfwWindowHint(GLFW_FOCUS_ON_SHOW, config.focus_on_show);
		// glfwWindowHint(GLFW_SCALE_TO_MONITOR, config.scale_to_monitor);

		// glfwWindowHint(GLFW_RED_BITS, config.red_bits);
		// glfwWindowHint(GLFW_GREEN_BITS, config.green_bits);
		// glfwWindowHint(GLFW_BLUE_BITS, config.blue_bits);
		// glfwWindowHint(GLFW_ALPHA_BITS, config.alpha_bits);
		// glfwWindowHint(GLFW_DEPTH_BITS, config.depth_bits);
		// glfwWindowHint(GLFW_STENCIL_BITS, config.stencil_bits);
		// glfwWindowHint(GLFW_ACCUM_RED_BITS, config.accum_red_bits);
		// glfwWindowHint(GLFW_ACCUM_GREEN_BITS, config.accum_green_bits);
		// glfwWindowHint(GLFW_ACCUM_BLUE_BITS, config.accum_blue_bits);
		// glfwWindowHint(GLFW_ACCUM_ALPHA_BITS, config.accum_alpha_bits);

		// glfwWindowHint(GLFW_AUX_BUFFERS, config.aux_buffers);
		// glfwWindowHint(GLFW_SAMPLES, config.samples);
		// glfwWindowHint(GLFW_REFRESH_RATE, config.refresh_rate);
		// glfwWindowHint(GLFW_STEREO, config.stereo);
		// glfwWindowHint(GLFW_SRGB_CAPABLE, config.srgb_capable);
		// glfwWindowHint(GLFW_DOUBLEBUFFER, config.double_buffer);

		// glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// window out{};
		// out.window_ = glfwCreateWindow(size.x, size.y, cstring_from_view(name), nullptr, nullptr);
		// glfwSetWindowUserPointer(out.window_, &out);
		// glfwSetFramebufferSizeCallback(out.window_, +[](GLFWwindow* window_, int width, int height){
		// 	window& window = *(struct window*)glfwGetWindowUserPointer(window_);
		// 	window.resized(window, {width, height});
		// });
		// return out;
		sdl.init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
		window out;
		out.window_ = SDL_CreateWindow(cstring_from_view(name), size.x, size.y, 0); // TODO: Check result
		if(!out.window_) STYLIZER_THROW(SDL_GetError());
		return out;
	}

	window& window::operator=(window&& o) {
		window_ = std::exchange(o.window_, nullptr);
		resized = std::move(o.resized);

		if(o.callback_id != std::numeric_limits<size_t>::max())
			sdl.event_handler.erase(sdl.event_handler.begin() + o.callback_id);
		callback_id = sdl.event_handler.size();
		sdl.event_handler.emplace_back([this](const SDL_Event& event){
			if(event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
				if(event.window.windowID == SDL_GetWindowID(window_))
					resized(*this, {event.window.data1, event.window.data2});

			if(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
				if(event.window.windowID == SDL_GetWindowID(window_))
					should_close_ = true;
		});
		return *this;
	}
	bool window::should_close(bool process_events /* = true */) const {
		if(process_events)
			sdl.poll_all();
		return should_close_;
	}
	uint2 window::get_dimensions() const {
		int x, y;
		if(!SDL_GetWindowSizeInPixels(window_, &x, &y))
			STYLIZER_THROW(SDL_GetError());
		return {x, y};
	}

#ifndef STYLIZER_USE_ABSTRACT_API
	context window::create_context(const api::device::create_config& config /* = {} */) {
		auto partial = context::create_default(config);
		partial.surface = &(surface = api::sdl3::create_surface<api::current_backend::surface>(window_));
		return partial;
	}
#endif

	void window::release() {
		surface.release();
		SDL_DestroyWindow(window_);
		window::~window();
	}

} // namespace stylizer