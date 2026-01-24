import std.compat;
import stylizer;
import stylizer.sdl3.input;

int main() {
	auto ctx = stylizer::context::create_default_with_error_handler();

	auto texture = stylizer::texture::create(ctx);

	stylizer::sdl3::input input;
	input.register_event_listener(ctx);
}