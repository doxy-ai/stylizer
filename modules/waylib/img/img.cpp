#include "img.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

STYLIZER_BEGIN_NAMESPACE
	template<typename F>
	image load_stb_image(const F& loader, bool hdr) {
		image image;
		vec2i size;
		int channels;
		image.data = {true, loader(&size.x, &size.y, &channels)};
		if(*image.data == nullptr)
			STYLIZER_THROW(stbi_failure_reason());

		image.format = hdr ? image_format::RGBA : image_format::RGBA8;
		image.size = size;
		image.frames = 1;
		return image;
	}

	image img::load(const std::filesystem::path& file_path) {
		if(stbi_is_hdr(file_path.c_str()))
			return load_stb_image([file_path](int* x, int* y, int* c) { return stbi_loadf(file_path.c_str(), x, y, c, 4); }, true);
		else return load_stb_image([file_path](int* x, int* y, int* c) { return stbi_load(file_path.c_str(), x, y, c, 4); }, false);
	}

	image img::load_from_memory(std::span<std::byte> data) {
		if(stbi_is_hdr_from_memory((stbi_uc*)data.data(), data.size()))
			return load_stb_image([data](int* x, int* y, int* c) { return stbi_loadf_from_memory((stbi_uc*)data.data(), data.size(), x, y, c, 4); }, true);
		else return load_stb_image([data](int* x, int* y, int* c) { return stbi_load_from_memory((stbi_uc*)data.data(), data.size(), x, y, c, 4); }, false);
	}

STYLIZER_END_NAMESPACE