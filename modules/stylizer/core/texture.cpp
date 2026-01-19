#include "api.hpp"

namespace stylizer {

	texture& texture::operator=(const texture& o) {
		*this = create(*o.creation_context, o.config, o.sample_config);
		copy_from(*creation_context, o);
		return *this;
	}

	texture& texture::operator=(texture&& o) {
		update_as_internal([&] {
			*((super*)this) = std::move((super&)o);
			config = o.config;
			sample_config = o.sample_config;
			creation_context = o.creation_context;
			size = std::move(o.size);
			o.resize.close();
			resize = reaction::action([this](const stdmath::vector<size_t, 3>& size) { resize_impl(size); }, size);
		});
		return *this;
	}

	texture texture::create(context& ctx, const create_config& config_ /* = {} */, const std::optional<sampler_config>& sampler /* = {} */) {
		auto config = config_;
		config.usage |= api::usage::RenderAttachment;

		texture out;
		out.update_as_internal([&] {
			out.config = config;
			out.sample_config = sampler;
			out.creation_context = &ctx;
			out.size = reaction::var(stdmath::vector<uint32_t, 3>{out.config.size});
			static_cast<api::current_backend::texture&>(out) = api::current_backend::texture::create(ctx, config);
			if(sampler) out.configure_sampler(ctx, *sampler);
			out.resize = reaction::action([self = &out](const stdmath::vector<size_t, 3>& size){
				self->resize_impl(size);
			}, out.size);
		});
		return out;
	}

	texture texture::create_and_write(context& ctx, std::span<const std::byte> data, const data_layout& layout, const create_config& config_ /* = {} */, const std::optional<sampler_config>& sampler /* = {} */) {
		create_config config = config_;
		config.size = { data.size() / layout.rows_per_image / bytes_per_pixel(config.format), layout.rows_per_image, 1 };
		config.usage |= api::usage::CopyDestination;
		auto out = create(ctx, config, sampler);
		out.write(ctx, data, layout, config.size);
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

	void texture::resize_impl(const stdmath::vector<uint32_t, 3>& size) {
		if(internal_update) return;

		config.size = size;
		auto new_text = api::current_backend::texture::create(*creation_context, config);
		if(sample_config && !is_depth_stencil(config.format)) new_text.blit_from(*creation_context, *this);
		if(sample_config) new_text.configure_sampler(*creation_context, *sample_config);
		// TODO: Is there a way to copy over the depth info? // TODO: Is this format error actually a problem with the blit shader?
		auto& old = static_cast<api::current_backend::texture&>(*this);
		old.release();
		old = std::move(new_text);
	}

}