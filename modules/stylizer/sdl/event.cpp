#include <SDL3/SDL_init.h>

#define STYLIZER_GLOBAL_SDL_EVENT_HANDLER_IMPLEMENTATION
#include "event.hpp"

namespace stylizer::sdl3 {

    struct api_manager {
        static api_manager& singleton() {
            static api_manager manager;
            return manager;
        }

		api_manager() {
			if (!SDL_Init(SDL_INIT_VIDEO))
				stylizer::get_error_handler()(api::error::severity::Error, "Failed to initialize SDL", 0);
		}
		~api_manager() {
			SDL_Quit();
		}
	};

    std::optional<connection>& get_global_sdl_event_handler() {
        static std::optional<connection> global_sdl_event_handler = []() -> std::optional<connection> {
            api_manager::singleton();
            return {};
        }();

        return global_sdl_event_handler;
    }

    connection_raw setup_sdl_events(context& ctx) {
        return ctx.process_events.connect([](context& ctx){
            for (SDL_Event event; SDL_PollEvent(&event); ) {
                struct event e;
                e.sdl = event;
                ctx.handle_event(e);
            }
        });
    }
}