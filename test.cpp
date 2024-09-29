#include <iostream>

#include "waylib/waylib.hpp"

int main() {
	wl::auto_release window = wl::window::create({800, 600});
	wl::auto_release state = window.create_default_state().throw_if_error();
	window.reconfigure_surface_on_resize(state);

	// GBuffer that gets its color format from the window
	auto gbuffer = wl::Gbuffer::create_default(state, window.get_size(), {
		.color_format = wgpu::TextureFormat::Undefined
	}).throw_if_error();
	window.raw.sizeEvent.append([&gbuffer, &state](auto& window, int x, int y) {
		gbuffer.resize(state, {x, y}).throw_if_error();
	});

	// WAYLIB_MAIN_LOOP(!window.should_close(),
	while(!window.should_close()) {
		auto draw = gbuffer.begin_drawing(state, {{.7, .1, .2, 1}}).throw_if_error();
		{

		}
		draw.draw().get().throw_if_error();

		// Blit texture
		auto surface = state.current_surface_texture().throw_if_error();
		gbuffer.color().blit_to(state, surface).throw_if_error();
		state.surface.present();
	}
	// );
}