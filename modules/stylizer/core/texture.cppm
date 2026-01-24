module;

#include "util/move_and_make_owned_macros.hpp"
#include "util/reaction.hpp"

export module stylizer:texture;

import :context;

import stdmath.slang;

namespace stylizer {
	using namespace magic_enum::bitwise_operators;

	export struct texture : public graphics::current_backend::texture { STYLIZER_MOVE_AND_MAKE_OWNED_METHODS(texture)
		using super = graphics::current_backend::texture;

		reaction::Var<stdmath::uint3> size;

		texture() = default;
		texture(const texture& o) { *this = o; }
		texture(texture&& o) { *this = std::move(o); }
		texture(super&& o) : super(std::move(o)) {}
		texture& operator=(const texture& o) {
			*this = create(*o.creation_context, o.config, o.sample_config);
			copy_from(*creation_context, o);
			return *this;
		}

		texture& operator=(texture&& o) {
			update_as_internal([&] -> void {
				*((super*)this) = std::move((super&)o);
				config = std::exchange(o.config, {});
				sample_config = std::exchange(o.sample_config, {});
				creation_context = std::exchange(o.creation_context, nullptr);
				size = std::move(o.size);
				o.resize.close();
				resize = reaction::action([this](const stdmath::uint3& size) { resize_impl(size); }, size);
			});
			return *this;
		}

		static stylizer::texture create(context& ctx, const create_config& config_ = {}, const std::optional<sampler_config>& sampler = {}) {			
			auto config = config_;
			config.usage |= graphics::usage::RenderAttachment;

			stylizer::texture out;
			out.update_as_internal([&] -> void {
				out.config = config;
				out.sample_config = sampler;
				out.creation_context = &ctx;
				out.size = reaction::var(stdmath::uint3{out.config.size});
				static_cast<graphics::current_backend::texture&>(out) = graphics::current_backend::texture::create(ctx, config);
				if(sampler) out.configure_sampler(ctx, *sampler);
				out.resize = reaction::action([self = &out](const stdmath::uint3& size){
					self->resize_impl(size);
				}, out.size);
			});
			return out;
		}

		static stylizer::texture create_and_write(context& ctx, std::span<const std::byte> data, const data_layout& layout, const create_config& config_ = {}, const std::optional<sampler_config>& sampler = {}) {
			create_config config = config_;
			config.size = { data.size() / layout.rows_per_image / bytes_per_pixel(config.format), layout.rows_per_image, 1 };
			config.usage |= graphics::usage::CopyDestination;
			auto out = create(ctx, config, sampler);
			out.write(ctx, data, layout, config.size);
			return out;
		}

		static stylizer::texture& get_default_texture(context& ctx) {
			static stylizer::texture global = [](context& ctx) -> stylizer::texture {
				std::array<stdmath::float4, 4> default_texture_data = {{
					{1, 0, 0, 1},
					{0, 1, 0, 1},
					{0, 0, 1, 1},
					{1, 1, 1, 1}
				}};
				stylizer::texture out = stylizer::texture::create_and_write(ctx, stylizer::byte_span<stdmath::float4>(default_texture_data), stylizer::graphics::texture::data_layout{
					.offset = 0,
					.bytes_per_row = sizeof(default_texture_data[0]) * 2,
					.rows_per_image = 2,
				}, {
					.format = graphics::texture::format::RGBA32
				});
				out.configure_sampler(ctx);
				return out;
			}(ctx);

			return global;
		}

		texture& configure_sampler(context& ctx, const sampler_config& config = {}) {
			sample_config = config;
			graphics::current_backend::texture::configure_sampler(ctx, config);
			return *this;
		}

		void release() {
			super::release();
			resize.close();
			size.close();
		}

	protected:
		bool internal_update = false;
		create_config config;
		std::optional<sampler_config> sample_config;
		context* creation_context;

		template<typename Tfunc>
		auto update_as_internal(const Tfunc& func) -> decltype(func()) {
			internal_update = true;
			if constexpr (std::is_same_v<decltype(func()), void>) {
				func();
				internal_update = false;
			} else {
				auto out = func();
				internal_update = false;
				return out;
			}
		}

		reaction::Action<> resize; void resize_impl(const stdmath::uint3& size)  {
			if(internal_update) return;

			config.size = size;
			auto new_text = graphics::current_backend::texture::create(*creation_context, config);
			if(sample_config && !is_depth_stencil(config.format)) new_text.blit_from(*creation_context, *this);
			if(sample_config) new_text.configure_sampler(*creation_context, *sample_config);
			// TODO: Is there a way to copy over the depth info? // TODO: Is this format error actually a problem with the blit shader?
			auto& old = static_cast<graphics::current_backend::texture&>(*this);
			old.release();
			old = std::move(new_text);
		}

		// Hide some of super's methods
		using super::create;
		using super::create_and_write;
		using super::configure_sampler;
	};
}