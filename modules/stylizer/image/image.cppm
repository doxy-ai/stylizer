module;

#include <stylizer/core/util/move_and_make_owned_macros.hpp>

export module stylizer.image:image;

import std.compat;
import stdmath.slang;
import stylizer;

namespace stylizer {
    using namespace magic_enum::bitwise_operators;

    export struct image { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(image)
		// For a 2D image the Z dimension is 1
		// The W dimension is bytes of color data
		using byte_grid = stdmath::stl::mdspan<std::byte, stdmath::stl::extents<size_t, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent>>;
		template<typename Tcolor>
		using pixel_grid = stdmath::stl::mdspan<Tcolor, stdmath::stl::extents<size_t, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent>>;

		static std::unordered_map<std::string, std::function<maybe_owned<image>(context&, std::span<std::byte>, std::string_view)>>& get_loader_set();
		static maybe_owned<image> load(context& ctx, std::filesystem::path file);

		virtual texture::format get_format() = 0;
		virtual byte_grid get_byte_grid() = 0;
		virtual size_t extent(size_t dimension) { return get_byte_grid().extent(dimension); }
		virtual size_t bytes_size() { return get_byte_grid().size();}
		virtual std::span<std::byte> get_pixel_bytes(size_t x, size_t y, size_t z = 0) {
			auto grid = get_byte_grid();
			auto bytes = stdmath::stl::submdspan(grid, x, y, z, stdmath::stl::full_extent);
			return {bytes.data_handle(), bytes.extent(0)};
		}

		virtual texture& upload(context& ctx, texture& texture, texture::create_config config_template = {}, const std::optional<texture::sampler_config>& sampler_config = {}) {
			auto byte_grid = get_byte_grid();
			stdmath::uint3 extents = {extent(0), extent(1), extent(2)};
			auto bytes_per_row = extents.x * extents.z * extent(3);
			auto rows_per_image = extents.y;
			auto size = bytes_per_row * rows_per_image;
			
			if(!texture || texture.texture_format() != get_format()) {
				if(texture) texture.release();
				config_template.format = get_format();
				config_template.size = extents;
				config_template.usage |= graphics::usage::CopyDestination;
				texture = texture::create(ctx, config_template, sampler_config);
			}
			texture.write(ctx, {byte_grid.data_handle(), size}, {
				.offset = 0,
				.bytes_per_row = bytes_per_row,
				.rows_per_image = rows_per_image
			}, extents);
			return texture;
		};
		texture upload(context& ctx, texture::create_config config_template = {}, const std::optional<texture::sampler_config>& sampler_config = {}) {
			texture out;
			return std::move(upload(ctx, out, config_template, sampler_config));
		}
	};

	export template<typename Tcolor>
	struct default_texture_format {
		static constexpr texture::format value = texture::format::RGBA32;
	};
	template<>
	struct default_texture_format<stdmath::float4> {
		static constexpr texture::format value = texture::format::RGBA32;
	};
	template<>
	struct default_texture_format<stdmath::byte4> {
		static constexpr texture::format value = texture::format::RGBA8srgb;
	};
	
	export template<typename Tcolor>
	constexpr texture::format default_texture_format_v = default_texture_format<Tcolor>::value;

}