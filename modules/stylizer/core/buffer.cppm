module;

#include "util/move_and_make_owned_macros.hpp"

export module stylizer:buffer;

import :context;

// import stdmath.slang;

namespace stylizer {
	export struct managed_buffer : public graphics::current_backend::buffer { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(managed_buffer)
		virtual std::span<const std::byte> to_bytes() = 0;
		virtual managed_buffer& upload(context& ctx, std::string_view label = "Stylizer Managed Buffer") {
			auto bytes = to_bytes();
			if(!*this || size() < bytes.size()) {
				auto usage = *this ? this->usage() : graphics::usage::Storage;
				static_cast<graphics::current_backend::buffer&>(*this) = ctx.create_and_write_buffer(usage, bytes, 0, label);
			} else
				write(ctx, bytes);
			return *this;
		}
	};

	export struct buffer : public graphics::current_backend::buffer {
		using managed = managed_buffer;
	};
}