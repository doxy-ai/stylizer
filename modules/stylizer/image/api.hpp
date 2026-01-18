#pragma once

#include "api.hpp"

#include <stylizer/core/api.hpp>
#include <stylizer/core/util/load_file.hpp>
#include <stylizer/core/util/maybe_owned.hpp>

namespace stylizer { inline namespace images {
	/**
	 * @brief Represents a generic image that can be loaded and uploaded to a texture.
	 */
	struct image {
		/**
		 * @brief For a 2D image the Z dimension is 1. The W dimension is bytes of color data.
		 */
		using byte_grid = stdmath::stl::mdspan<std::byte, stdmath::stl::extents<size_t, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent>>;

		/**
		 * @brief A grid of pixels of type Tcolor.
		 *
		 * @tparam Tcolor The type representing a single pixel's color.
		 */
		template<typename Tcolor>
		using pixel_grid = stdmath::stl::mdspan<Tcolor, stdmath::stl::extents<size_t, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent, stdmath::stl::dynamic_extent>>;

		/**
		 * @brief Gets the set of registered image loaders.
		 * @return A reference to the loader set map.
		 */
		static std::unordered_map<std::string, std::function<maybe_owned<image>(context&, std::span<std::byte>, std::string_view)>>& get_loader_set();

		/**
		 * @brief Loads an image from a file.
		 * @param ctx The stylizer context.
		 * @param file The path to the image file.
		 * @return A maybe_owned pointer to the loaded image.
		 *
		 * @code
		 * auto img = stylizer::image::load(ctx, "path/to/image.png");
		 * if (img) {
		 *     auto tex = img->upload(ctx);
		 * }
		 * @endcode
		 */
		static maybe_owned<image> load(context& ctx, std::filesystem::path file);

		/**
		 * @brief Gets the texture format that best represents this image.
		 * @return The texture format.
		 */
		virtual texture::format get_format() = 0;

		/**
		 * @brief Gets the underlying byte grid of the image.
		 * @return The byte grid.
		 */
		virtual byte_grid get_byte_grid() = 0;

		/**
		 * @brief Gets the extent of the image in a specific dimension.
		 * @param dimension The dimension index.
		 * @return The extent in that dimension.
		 */
		virtual size_t extent(size_t dimension) { return get_byte_grid().extent(dimension); }

		/**
		 * @brief Gets the total size of the image in bytes.
		 * @return The total byte size.
		 */
		virtual size_t bytes_size() { return get_byte_grid().size();}

		/**
		 * @brief Gets the bytes of a specific pixel.
		 * @param x The x-coordinate.
		 * @param y The y-coordinate.
		 * @param z The z-coordinate (default 0).
		 * @return A span of bytes representing the pixel.
		 */
		virtual std::span<std::byte> get_pixel_bytes(size_t x, size_t y, size_t z = 0) {
			auto grid = get_byte_grid();
			auto bytes = stdmath::submdspan(grid, x, y, z, stdmath::full_extent);
			return {bytes.data_handle(), bytes.extent(0)};
		}

		/**
		 * @brief Uploads the image data to a texture.
		 * @param ctx The stylizer context.
		 * @param texture The texture to upload to. If it's null or has wrong format, it will be (re)created.
		 * @param config_template Configuration template for texture creation.
		 * @return Reference to the uploaded texture.
		 *
		 * @code
		 * stylizer::texture tex;
		 * img->upload(ctx, tex);
		 * @endcode
		 */
		virtual texture& upload(context& ctx, texture& texture, texture::create_config config_template = {}) {
			auto byte_grid = get_byte_grid();
			stdmath::vector<size_t, 3> extents = {extent(0), extent(1), extent(2)};
			auto bytes_per_row = extents.x * extents.z * extent(3);
			auto rows_per_image = extents.y;
			auto size = bytes_per_row * rows_per_image;
			
			if(!texture || texture.texture_format() != get_format()) {
				if(texture) texture.release();
				config_template.format = get_format();
				config_template.size = extents;
				config_template.usage |= api::usage::CopyDestination;
				texture = texture::create(ctx, config_template);
			}
			texture.write(ctx, {byte_grid.data_handle(), size}, {
				.offset = 0,
				.bytes_per_row = bytes_per_row,
				.rows_per_image = rows_per_image
			}, extents);
			return texture;
		};

		/**
		 * @brief Uploads the image to a new texture.
		 * @param ctx The stylizer context.
		 * @param config_template Configuration template for texture creation.
		 * @return The new texture.
		 *
		 * @code
		 * auto tex = img->upload(ctx);
		 * @endcode
		 */
		texture upload(context& ctx, texture::create_config config_template = {}) {
			texture out;
			return std::move(upload(ctx, out, config_template));
		}

		/** @brief Virtual destructor. */
		virtual ~image() = default;
	};

	/**
	 * @brief Maps a color type to its default texture format.
	 *
	 * @tparam Tcolor The color type.
	 */
	template<typename Tcolor>
	struct default_texture_format {
		/**
		 * @brief The default texture format for type Tcolor.
		 */
		static constexpr texture::format value = texture::format::RGBA32;
	};

	/**
	 * @brief Specialization for float4 vectors.
	 */
	template<>
	struct default_texture_format<stdmath::vector<float, 4>> {
		static constexpr texture::format value = texture::format::RGBA32;
	};

	/**
	 * @brief Specialization for uint8_t4 vectors.
	 */
	template<>
	struct default_texture_format<stdmath::vector<uint8_t, 4>> {
		static constexpr texture::format value = texture::format::RGBA8srgb;
	};
	
	/**
	 * @brief Helper constant for default texture format.
	 *
	 * @tparam Tcolor The color type.
	 */
	template<typename Tcolor>
	constexpr static texture::format default_texture_format_v = default_texture_format<Tcolor>::value;

	/**
	 * @brief Generic loader using stb_image.
	 * @param ctx The stylizer context.
	 * @param memory The memory buffer containing the image data.
	 * @param extension The file extension/format.
	 * @return A maybe_owned image.
	 *
	 * @code
	 * auto img = stylizer::load_stb_image_generic(ctx, data_span, "png");
	 * @endcode
	 */
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
