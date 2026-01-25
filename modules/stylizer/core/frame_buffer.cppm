module;

#include "util/move_and_make_owned_macros.hpp"
#include "util/reaction.hpp"

export module stylizer:frame_buffer;

// import :context;
import :time;
// import :texture;
import :surface;

// import stdmath;
// import stdmath.slang;

namespace stylizer {
	
	export struct frame_buffer { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(frame_buffer)
		reaction::Var<stdmath::uint3> size;
		std::optional<stdmath::float4> clear_value = {};

		virtual ~frame_buffer() {}

		virtual texture& color_texture() = 0;
		virtual const texture& color_texture() const { return const_cast<frame_buffer*>(this)->color_texture(); }
		virtual std::span<const graphics::color_attachment> color_attachments(graphics::color_attachment attachment_template = {}) const = 0;
		virtual std::optional<graphics::depth_stencil_attachment> depth_stencil_attachment(graphics::depth_stencil_attachment attachment_template = {}) const { return {}; }

		virtual graphics::current_backend::render::pass create_render_pass(context& ctx, graphics::color_attachment color_template = {}, graphics::depth_stencil_attachment depth_template = {}, bool one_shot = true) {
			return ctx.create_render_pass(color_attachments(color_template), depth_stencil_attachment(depth_template), one_shot);
		}

		// virtual frame_buffer& update_size_debounced(const stdmath::uint3& size, float dt, float time_to_wait = .1){
		// 	struct debounce_state {
		// 		stdmath::uint3 last;
		// 		float timer = std::numeric_limits<float>::infinity();
		// 	};
		// 	static std::unordered_map<frame_buffer*, debounce_state> states;
		// 	auto& state = states[this];

		// 	// Only update when size stops changing!
		// 	if(size - state.last == stdmath::uint3{0, 0})
		// 		state.timer -= dt;
		// 	else state.timer = time_to_wait;

		// 	if(state.timer < 0)
		// 		update_if_any_different(this->size, size);

		// 	state.last = size;
		// 	return *this;
		// }
		// frame_buffer& update_size_debounced(const stdmath::uint3& size, struct time& time, float time_to_wait = .1) {
		// 	return update_size_debounced(size, time.delta, time_to_wait);
		// }

		// frame_buffer& update_size_from_surface(surface& surface, struct time& time, float time_to_wait = .1) {
		// 	return update_size_debounced(surface.texture_size()(), time, time_to_wait);
		// }
		// frame_buffer& update_size_from_texture(texture& texture, struct time& time, float time_to_wait = .1) {
		// 	return update_size_debounced(texture.size(), time, time_to_wait);
		// }

		// virtual reaction::Action<> link_size_to_surface(surface& surface) {
		// 	return reaction::action([this](const stdmath::uint3& size) {
		// 		this->size.value(size);
		// 	}, surface.texture_size());
		// }

		constexpr static graphics::depth_stencil_attachment default_draw_to_depth_config {
			.depth_clear_value = 1,
			.depth_comparison_function = graphics::comparison_function::Less
		};

		template<typename Tfunc>
		auto&& draw_to(context& ctx, const Tfunc& func, graphics::color_attachment color_template = {}, graphics::depth_stencil_attachment depth_template = default_draw_to_depth_config) {
			auto pass = create_render_pass(ctx, color_template, depth_template);
			if constexpr (std::is_same_v<decltype(func(pass)), void>) {
				func(pass);
				pass.one_shot_submit(ctx);
				return *this;
			} else {
				auto out = func(pass);
				pass.one_shot_submit(ctx);
				return out;
			}
		}
	};

}