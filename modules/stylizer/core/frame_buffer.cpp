#include "api.hpp"

namespace stylizer {

	frame_buffer& frame_buffer::update_size_debounced(const stdmath::vector<uint32_t, 3>& size, float dt, float time_to_wait /* = .1 */) {
		struct debounce_state {
			stdmath::vector<uint32_t, 3> last;
			float timer = std::numeric_limits<float>::infinity();
		};
		static std::unordered_map<frame_buffer*, debounce_state> states;
		auto& state = states[this];

		// Only update when size stops changing!
		if(size - state.last == stdmath::vector<uint32_t, 3>{0, 0})
			state.timer -= dt;
		else state.timer = time_to_wait;

		if(state.timer < 0)
			update_if_any_different(this->size, size);

		state.last = size;
		return *this;
	}

	frame_buffer& frame_buffer::update_size_debounced(const stdmath::vector<uint32_t, 3>& size, struct time& time, float time_to_wait /* = .1 */) {
		return update_size_debounced(size, time.delta, time_to_wait);
	}

}