#pragma once

#include "api.hpp"

#include <optional>
#include <stylizer/core/api.hpp>
#include <stylizer/core/util/load_file.hpp>
#include <stylizer/core/util/maybe_owned.hpp>

namespace stylizer { inline namespace images {
	struct image { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(image)
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
			auto bytes = stdmath::submdspan(grid, x, y, z, stdmath::full_extent);
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
				config_template.usage |= api::usage::CopyDestination;
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

	template<typename Tcolor>
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
	
	template<typename Tcolor>
	constexpr static texture::format default_texture_format_v = default_texture_format<Tcolor>::value;

	stylizer::maybe_owned<stylizer::image> load_stb_image_generic(context&, std::span<std::byte> memory, std::string_view extension);

	// struct animated_image : public image {
	// 	using byte_grid = image::byte_grid;
	// 	template<typename Tcolor>
	// 	using pixel_grid = image::pixel_grid<Tcolor>;

	// 	size_t current_frame = 0;
	// 	float current_time_till_next_frame;

	// 	virtual byte_grid get_byte_grid(size_t frame) = 0;
	// 	virtual size_t extent(size_t frame, size_t dimension) { return get_byte_grid(frame).extent(dimension); }
	// 	virtual size_t bytes_size(size_t frame) { return get_byte_grid(frame).size();}
	// 	virtual std::span<std::byte> get_pixel_bytes(size_t frame, size_t x, size_t y, size_t z = 0) {
	// 		auto grid = get_byte_grid(frame);
	// 		auto bytes = stdmath::submdspan(grid, x, y, z, stdmath::full_extent);
	// 		return {bytes.data_handle(), bytes.extent(0)};
	// 	}

	// 	byte_grid get_byte_grid() override { 
	// 		return get_byte_grid(current_frame);
	// 	}

	// 	virtual void restart() {
	// 		current_frame = 0;
	// 		current_time_till_next_frame = total_time_till_next_frame(0);
	// 	}
	// 	virtual void update(float dt) {
	// 		current_time_till_next_frame -= dt;
	// 		if(current_time_till_next_frame < 0)
	// 			current_time_till_next_frame += total_time_till_next_frame(++current_frame);
	// 			// TODO: Next frame event
	// 	}

	// 	virtual size_t total_frame_count() = 0;
	// 	virtual float total_time_till_next_frame(size_t frame) = 0;
	// 	virtual float total_length() {
	// 		float accumulate = 0;
	// 		for(size_t frame = 0, total = total_frame_count(); frame < total; ++frame)
	// 			accumulate += total_time_till_next_frame(frame);
	// 		return accumulate;
	// 	}
	// };

	// struct animated_image_sequence : public animated_image {
	// 	std::vector<std::unique_ptr<image>> sequence;
	// 	float framerate = 1.0/60;

	// 	byte_grid get_byte_grid(size_t frame) override { return sequence[frame]->get_byte_grid(); }
	// 	size_t total_frame_count() override { return sequence.size(); }
	// 	float total_time_till_next_frame(size_t frame) override { return framerate; }
	// 	float total_length() override { return total_frame_count() * framerate; }
	// };
}}
