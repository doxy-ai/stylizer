module;

#include "util/move_and_make_owned_macros.hpp"

export module stylizer:camera;

import :context;

import stdmath.slang;

namespace stylizer {
	export struct concrete_camera;

	export struct camera { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(camera)
		virtual stdmath::float4x4 view_matrix() const = 0;
		virtual stdmath::float4x4 projection_matrix(const stdmath::uint2& screen_size) const = 0;

		virtual stdmath::float4x4 inverse_view_matrix() const {
			return inverse(view_matrix());
		}
		virtual stdmath::float4x4 inverse_projection_matrix(const stdmath::uint2& screen_size) const {
			return inverse(projection_matrix(screen_size));
		}

		using concrete = concrete_camera;
	};

	struct concrete_camera : public camera { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(concrete_camera, camera)
		stdmath::float4x4 view, projection;
		stdmath::float4x4 inverse_view, inverse_projection;

		static concrete_camera from_camera(const camera& camera, const stdmath::uint2& screen_size) {
			concrete_camera out;
			out.view = camera.view_matrix();
			out.projection = camera.projection_matrix(screen_size);
			out.inverse_view = camera.inverse_view_matrix();
			out.inverse_projection = camera.inverse_projection_matrix(screen_size);
			return out;
		}

		stdmath::float4x4 view_matrix() const override { return view; }
		stdmath::float4x4 projection_matrix(const stdmath::uint2& screen_size = {}) const override { return projection; }
		stdmath::float4x4 inverse_view_matrix() const override { return inverse_view; }
		stdmath::float4x4 inverse_projection_matrix(const stdmath::uint2& screen_size = {}) const override { return inverse_projection; }
	};
}