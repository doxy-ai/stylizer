#include "input.hpp"
#include "event.hpp"

namespace stylizer::sdl {

	std::optional<connection>& get_global_sdl_event_handler(); // defined in core/util/sdl_event.cpp

	void input::register_event_listener(context& ctx) {
		if(!get_global_sdl_event_handler())
			get_global_sdl_event_handler() = setup_sdl_events(ctx);

		ctx.handle_event.connect([this](const context::event& e) {
			auto event = event2sdl(e);
			if(!event) return;

			process_SDL_event(event->sdl);
		});
	}

	input::boolean convert_keycode(SDL_Keycode key) {
		return (input::boolean)key;
	}

	bool input::process_SDL_event(const SDL_Event& event) {
		auto processed = false;
		reaction::batchExecute([&] -> void {
			switch(event.type) {
			break; case SDL_EVENT_KEY_DOWN: [[fallthrough]];
			case SDL_EVENT_KEY_UP: {
				update_if_different(get_integer(current_device), int64_t(event.key.which));

				auto lookup = convert_keycode(event.key.key);
				if(booleans.contains(lookup)) {
					auto& map = booleans[lookup];
					if(map.contains(input::all_devices)) {
						update_if_different(map[all_devices], event.key.down);
						processed = true;
					}

					device_t specific = event.key.which;
					if(map.contains(specific)) {
						update_if_different(map[specific], event.key.down);
						processed = true;
					}
				}
			}

			break; case SDL_EVENT_MOUSE_WHEEL: {
				update_if_different(get_integer(current_device), int64_t(event.wheel.which));

				if(vectors.contains(mouse_wheel)) {
					auto& map = vectors[mouse_wheel];
					if(map.contains(input::all_devices)) {
						update_if_any_different(map[all_devices], stdmath::float2{event.wheel.x, event.wheel.y});
						processed = true;
					}

					device_t specific = event.wheel.which;
					if(map.contains(specific)) {
						update_if_any_different(map[specific], stdmath::float2{event.wheel.x, event.wheel.y});
						processed = true;
					}
				}
			}

			break; case SDL_EVENT_MOUSE_MOTION: {
				update_if_different(get_integer(current_device), int64_t(event.motion.which));

				if(vectors.contains(mouse_position)) {
					auto& map = vectors[mouse_position];
					if(map.contains(input::all_devices)) {
						update_if_any_different(map[all_devices], stdmath::float2{event.motion.x, event.motion.y});
						processed = true;
					}

					device_t specific = event.motion.which;
					if(map.contains(specific)) {
						update_if_any_different(map[specific], stdmath::float2{event.motion.x, event.motion.y});
						processed = true;
					}
				}
			}
			break; default: break;
			}
		});

		return processed;
	}

}