export module stylizer:time;

import std.compat;

namespace stylizer {
	
	export struct time {
		float total = 0, delta = 0, smoothed_delta = 0;
		uint32_t frame = 0;

		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& compute_delta() {
			static auto last_time = Tclock::now();
			auto now = Tclock::now();
			auto dt_precise = std::chrono::duration_cast<std::chrono::duration<int64_t, Tprecision>>(now - last_time).count();
			delta = double(dt_precise * Tprecision::num) / Tprecision::den;
			last_time = now;
			return *this;
		}

		inline time& increment_frame() {
			++frame;
			return *this;
		}

		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& compute_delta_and_frame() {
			compute_delta<Tprecision, Tclock>();
			increment_frame();
			return *this;
		}

		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& per_frame(float alpha = .9) {
			compute_delta_and_frame<Tprecision, Tclock>();
			total += delta;
			smoothed_delta = alpha * smoothed_delta + (1 - alpha) * delta;
			return *this;
		}
	};

}