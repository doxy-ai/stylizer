module;

#include "util/move_and_make_owned_macros.hpp"

export module stylizer:utility_buffer;

import :camera;
import :time;
import :buffer;

// import stdmath.slang;

namespace stylizer {
	export struct utility_buffer : public buffer::managed { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(utility_buffer, managed_buffer)
		stylizer::camera::concrete camera;
		stylizer::time time;

		std::span<const std::byte> to_bytes() override {
			// NOTE: concrete camera has a vtable so its uploadable data starts at camera.view
			return {(std::byte*)&camera.view, ((std::byte*)&time) - ((std::byte*)&camera.view) + sizeof(time)};
		}

		utility_buffer& update(context& ctx, float alpha = .9) {
			time.update(alpha);
			upload(ctx, "Stylizer Utility Buffer");
			return *this;
		}
	};
}