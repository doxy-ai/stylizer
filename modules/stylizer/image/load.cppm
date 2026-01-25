module;

#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.hpp"

export module stylizer.image:load;

import std.compat;
import stdmath.slang;
import stylizer;
import stylizer.load_file;

import :image;
import :memory;

namespace stylizer { inline namespace images {
	export stylizer::dynamic_memory_image<stdmath::byte4> load_stb_image(context&, std::span<std::byte> memory, std::string_view extension /* = {} */) {
		int x, y, n;
		auto data = stbi_load_from_memory((uint8_t*)memory.data(), memory.size(), &x, &y, &n, 4);
		std::span<stdmath::byte4> span; std::vector<stdmath::byte4> tmp;
		if(n == 4)
			span = {(stdmath::byte4*)data, static_cast<size_t>(x * y)};
		else if(n == 3) {
			std::span<stdmath::byte3> span3 = {(stdmath::byte3*)data, static_cast<size_t>(x * y)};
			tmp.reserve(span3.size());
			for(auto& vec: span3)
				tmp.emplace_back(vec, std::byte{255}); // If there is no alpha data we set alpha to max
			span = tmp;
		}

		return {span, stylizer::dynamic_memory_image<uint8_t>::extents_t(x, y, 1)};
	}

	export stylizer::maybe_owned<stylizer::image> load_stb_image_generic(context& ctx, std::span<std::byte> memory, std::string_view extension /* = {} */) {
		return load_stb_image(ctx, memory, extension).move_to_owned();
	}

    std::unordered_map<std::string, std::function<maybe_owned<image>(context&, std::span<std::byte>, std::string_view)>>& image::get_loader_set() {
		static std::unordered_map<std::string, std::function<maybe_owned<image>(context&, std::span<std::byte>, std::string_view)>> loaders = []{
			std::unordered_map<std::string, std::function<maybe_owned<image>(context&, std::span<std::byte>, std::string_view)>> out;
			out[".png"] = out[".jpg"] = out[".jpeg"] = out[".tga"] 
				= out[".bmp"] = out[".psd"] = out[".pic"] = out[".pnm"] = load_stb_image_generic;
			return out;
		}();
		return loaders;
	}
	maybe_owned<image> image::load(context& ctx, std::filesystem::path file) {
		return load_file(ctx, file, get_loader_set()[file.extension().string()]);
	}
}}