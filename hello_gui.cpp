#include <stylizer/core/util/reaction.hpp>

import std.compat;
import stylizer;
import stylizer.sdl3.window;

int main() {
	auto ctx = stylizer::context::create_default_with_error_handler();

	auto window = stylizer::sdl3::window::create(ctx, "Hello GUI", {800, 600});
	window.register_event_listener(ctx);

	while(!window.close_requested()) {
		ctx.update();
		window.update();

		window.next_texture(ctx);
		window.present(ctx);
	}
}