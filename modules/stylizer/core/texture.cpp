#include "api.hpp"

namespace stylizer {

	texture texture::create(api::device& device, const create_config& config /* = {} */) {
		texture out;
		out.values = config;
		out.size = reaction::var(out.values.size);
		static_cast<api::current_backend::texture&>(out) = api::current_backend::texture::create(device, config);
		out.resize = reaction::action([](const stdmath::vector<size_t, 3>& size){
			// TODO: Create new texture and blit the current texture to it
		}, out.size);
		return out;
	}
	texture texture::create_and_write(api::device& device, std::span<const std::byte> data, const data_layout& layout, create_config config /* = {} */) {
		config.size = { data.size() / layout.rows_per_image / bytes_per_pixel(config.format), layout.rows_per_image, 1 };
		config.usage |= api::usage::CopyDestination;
		auto out = create(device, config);
		out.write(device, data, layout, config.size);
		return out;
	}

	texture& texture::get_default_texture(context& ctx) {
		static texture global = [](context& ctx) -> texture {
			std::array<stdmath::vector<float, 4>, 4> default_texture_data = {{
				{1, 0, 0, 1},
				{0, 1, 0, 1},
				{0, 0, 1, 1},
				{1, 1, 1, 1}
			}};
			auto out = stylizer::texture::create_and_write(ctx, stylizer::byte_span<stdmath::vector<float, 4>>(default_texture_data), stylizer::api::texture::data_layout{
				.offset = 0,
				.bytes_per_row = sizeof(default_texture_data[0]) * 2,
				.rows_per_image = 2,
			}, {
				.format = api::texture::format::RGBA32
			});
			out.configure_sampler(ctx);
			return out;
		}(ctx);

		return global;
	}

}