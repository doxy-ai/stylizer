#pragma once

#include "common.hpp"

#include <stylizer/input/api.hpp>

union SDL_Event;

namespace stylizer::sdl {

	struct input: public stylizer::input {
		size_t type = magic_number;

		void register_event_listener(context& ctx) override;

		std::unordered_map<boolean, std::unordered_map<device_t, reaction::Var<bool>>> booleans;

		reaction::Var<bool>& get_boolean(boolean what, device_t devices = all_devices) override {
			auto& map = booleans[what];
			if(!map.contains(devices))
				return map[devices] = reaction::var(false);
			else return map[devices];
		}

		std::unordered_map<floating, std::unordered_map<device_t, reaction::Var<float>>> floats;

		reaction::Var<float>& get_floating(floating what, device_t devices = all_devices) override {
			auto& map = floats[what];
			if(!map.contains(devices))
				return map[devices] = reaction::var(0.0f);
			else return map[devices];
		}

		std::unordered_map<integer, std::unordered_map<device_t, reaction::Var<int64_t>>> ints;

		reaction::Var<int64_t>& get_integer(integer what, device_t devices = all_devices) override {
			auto& map = ints[what];
			if(!map.contains(devices))
				return map[devices] = reaction::var(int64_t{0});
			else return map[devices];
		}

		std::unordered_map<vector, std::unordered_map<device_t, reaction::Var<stdmath::float2>>> vectors;

		reaction::Var<stdmath::float2>& get_vector(vector what, device_t devices = all_devices) override {
			auto& map = vectors[what];
			if(!map.contains(devices))
				return map[devices] = reaction::var(stdmath::float2{});
			else return map[devices];
		}

		bool process_SDL_event(const SDL_Event& event);
	};

	static_assert(input_concept<input>);

}
