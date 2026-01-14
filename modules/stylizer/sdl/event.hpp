#pragma once

#include "common.hpp"

#include <stylizer/core/api.hpp>
#include <SDL3/SDL_events.h>

namespace stylizer::sdl {
    struct event: public stylizer::context::event {
        size_t type = magic_number;
        SDL_Event sdl;
    };

    std::optional<connection>& get_global_sdl_event_handler();

    connection_raw setup_sdl_events(context& ctx);
}