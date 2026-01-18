#include "api.hpp"

namespace stylizer {

	api::current_backend::bind_group instance_data::buffer_base::make_bind_group(context& ctx,api::current_backend::render_pipeline& pipeline, std::optional<utility_buffer> util /* = {} */, size_t index /* = 0 */, size_t minimum_util_size /* = 272 */) {
		utility_buffer* cacher = util ? &*util : nullptr;
		if(group_cache.contains(cacher)) return group_cache[cacher];

		std::array<api::bind_group::binding, 2> bindings;
		bindings[0] = api::bind_group::buffer_binding{&*this};
		if(util) bindings[1] = api::bind_group::buffer_binding{&*util};
		else bindings[1] = api::bind_group::buffer_binding{&ctx.get_zero_buffer_singleton(api::usage::Storage, minimum_util_size)};
		return group_cache[cacher] = pipeline.create_bind_group(ctx, index, bindings);
	}

}