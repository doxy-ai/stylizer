module;

#include <stylizer/core/util/move_and_make_owned_macros.hpp>

export module stylizer.image:memory;

import std.compat;
import stdmath.slang;
import stylizer;

import :image;

namespace stylizer {
	
    export template<typename Tcolor, size_t X, size_t Y, size_t Z = 1>
	struct static_memory_image : public image { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(static_memory_image, image)
		std::array<std::byte, X * Y * Z * sizeof(Tcolor)> data;
		using pixel_grid = image::pixel_grid<Tcolor>;
		texture::format format = default_texture_format_v<Tcolor>;

		texture::format get_format() override { return format; }
		byte_grid get_byte_grid() override { return byte_grid{data.data(), X, Y, Z, sizeof(Tcolor)}; }
		pixel_grid get_pixel_grid() { return pixel_grid{(Tcolor*)data.data(), X, Y, Z}; }
		Tcolor& get_pixel(size_t x, size_t y, size_t z = 0) { return *(Tcolor*)get_pixel_bytes(x, y, z).data(); }
	};

	export template<typename Tcolor>
	struct dynamic_memory_image : public image { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(dynamic_memory_image, image)
		std::vector<std::byte> data;
		using extents_t = stdmath::stl::extents<size_t, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent>;
		extents_t extents;
		using pixel_grid = image::pixel_grid<Tcolor>;
		texture::format format = default_texture_format_v<Tcolor>;

		dynamic_memory_image(extents_t extents) : extents(extents) { data.resize(extents.extent(0) * extents.extent(1) * extents.extent(2)); }
		dynamic_memory_image(std::span<Tcolor> data, extents_t extents) : data((std::byte*)data.data(), ((std::byte*)data.data()) + data.size() * sizeof(Tcolor)), extents(extents) {
			assert(data.size() * sizeof(data[0]) == bytes_size());
		}

		texture::format get_format() override { return format; }
		byte_grid get_byte_grid() override { return byte_grid{data.data(), extents.extent(0), extents.extent(1), extents.extent(2), sizeof(Tcolor)}; }
		pixel_grid get_pixel_grid() { return pixel_grid{(Tcolor*)data.data(), extents.extent(0), extents.extent(1), extents.extent(2)}; }
		Tcolor& get_pixel(size_t x, size_t y, size_t z = 0) { return *(Tcolor*)get_pixel_bytes(x, y, z).data(); }
	};

}