#pragma once

#include "storage.hpp"

namespace stylizer { inline namespace models {

	struct instance_data {
		stdmath::float4x4 model = stdmath::float4x4::identity();

		struct buffer_base : public managed_buffer {
			std::unordered_map<utility_buffer*, graphics::current_backend::bind_group> group_cache;

			graphics::current_backend::bind_group make_bind_group(context& ctx, graphics::current_backend::render_pipeline& pipeline, std::optional<utility_buffer> util = {}, size_t index = 0, size_t minimum_util_size = 272) {
				utility_buffer* cacher = util ? &*util : nullptr;
				if(group_cache.contains(cacher)) return group_cache[cacher];

				std::array<graphics::bind_group::binding, 2> bindings;
				bindings[0] = graphics::bind_group::buffer_binding{&*this};
				if(util) bindings[1] = graphics::bind_group::buffer_binding{&*util};
				else bindings[1] = graphics::bind_group::buffer_binding{&ctx.get_zero_buffer_singleton(graphics::usage::Storage, minimum_util_size)};
				return group_cache[cacher] = pipeline.create_bind_group(ctx, index, bindings);
			}

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
