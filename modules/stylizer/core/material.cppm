module;

#include "util/move_and_make_owned_macros.hpp"

export module stylizer:material;

// import :context;
import :frame_buffer;

// import stdmath.slang;

namespace stylizer {

	export struct material : public graphics::current_backend::render::pipeline { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(material)
		material() = default; // TODO: Default materials okay?
		material(graphics::current_backend::render::pipeline&& pipeline)
			: graphics::current_backend::render::pipeline(std::move(pipeline)) {}

		material(const material&) = default;
		material(material&) = default;
		virtual ~material() {}

		material& operator=(graphics::current_backend::render::pipeline&& pipeline) {
			static_cast<graphics::current_backend::render::pipeline&>(*this) = std::move(pipeline);
			return *this;
		}
		material& operator=(const material&) = default;
		material& operator=(material&) = default;


		virtual std::span<maybe_owned<graphics::current_backend::texture>> textures(context& ctx) = 0;
		virtual std::span<maybe_owned<graphics::current_backend::buffer>> buffers(context& ctx) = 0;
		virtual std::span<std::string_view> requested_mesh_attributes() = 0;

		virtual std::span<graphics::current_backend::bind_group> bind_groups(context& ctx) = 0;

		virtual material& upload(context& ctx) = 0;
	};

	export template<typename T>
	concept material_concept = std::derived_from<T, material> && requires (T t, context ctx, frame_buffer fb) {
		{ T::create(ctx, fb) } -> std::convertible_to<T>;
	};

}