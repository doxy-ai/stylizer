module;

// #include <stylizer/core/util/reaction.hpp>
#include <stylizer/core/util/move_and_make_owned_macros.hpp>

export module stylizer.model:instance_data;

import std.compat;
// import stdmath;
import stdmath.slang;
import stylizer;

namespace stylizer { inline namespace models {

	export struct instance_data {
		stdmath::float4x4 model = stdmath::float4x4::identity();

		struct buffer_base : public managed_buffer {
			std::unordered_map<utility_buffer*, graphics::current_backend::bind_group> group_cache;

			graphics::current_backend::bind_group make_bind_group(context& ctx, graphics::current_backend::render_pipeline& pipeline, std::optional<utility_buffer> util = {}, size_t index = 0, size_t minimum_util_size = 272);

			virtual size_t count() const = 0;

			virtual void rebuild_gpu_caches() {
				group_cache.clear();
			}
		};

		template<size_t N = std::dynamic_extent>
		struct buffer : public buffer_base, public std::array<instance_data, N> { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(buffer, managed_buffer)
			using base = instance_data::buffer_base;

			size_t count() const override { return N; }

			std::span<const std::byte> to_bytes() override {
				return byte_span<instance_data>(static_cast<std::array<instance_data, N>&>(*this));
			}
		};
	};

	template<>
	struct instance_data::buffer<std::dynamic_extent> : public instance_data::buffer_base, public std::vector<instance_data> { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(buffer, managed_buffer)
		using base = instance_data::buffer_base;

		size_t count() const override { return std::vector<instance_data>::size(); }

		std::span<const std::byte> to_bytes() override {
			return byte_span<instance_data>(static_cast<std::vector<instance_data>&>(*this));
		}
	};

}}
