#include "img.hpp"
#include "stylizer/api/api.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

namespace stylizer::img {

	template<typename F>
	core::image load_stb_image(const F& loader, bool hdr) {
		core::image image;
		int x, y;
		int channels;
		std::byte* raw_data = (std::byte*)loader(&x, &y, &channels);
		// assert(channels == 4);
		if(raw_data == nullptr)
			STYLIZER_THROW(stbi_failure_reason());

		void* end = hdr ? raw_data + (x * y * sizeof(api::color32)) : raw_data + (x * y * sizeof(api::color8));
		image.raw_data = hdr ? core::image::data_t{std::vector<api::color32>((api::color32*)raw_data, (api::color32*)end)}
			: core::image::data_t{std::vector<api::color8>((api::color8*)raw_data, (api::color8*)end)};
		image.format = hdr ? api::texture::format::RGBA32 : api::texture::format::RGBA8;
		image.size.x = x;
		image.size.y = y;
		image.frames = 1;
		return image;
	}

	core::image load(const std::filesystem::path& file_path) {
		if(stbi_is_hdr(file_path.c_str()))
			return load_stb_image([&file_path](int* x, int* y, int* c) { return stbi_loadf(file_path.c_str(), x, y, c, 4); }, true);
		else return load_stb_image([&file_path](int* x, int* y, int* c) { return stbi_load(file_path.c_str(), x, y, c, 4); }, false);
	}

	core::image load_from_memory(std::span<const std::byte> data) {
		if(stbi_is_hdr_from_memory((stbi_uc*)data.data(), data.size()))
			return load_stb_image([data](int* x, int* y, int* c) { return stbi_loadf_from_memory((stbi_uc*)data.data(), data.size(), x, y, c, 4); }, true);
		else return load_stb_image([data](int* x, int* y, int* c) { return stbi_load_from_memory((stbi_uc*)data.data(), data.size(), x, y, c, 4); }, false);
	}

} // namespace stylizer::img