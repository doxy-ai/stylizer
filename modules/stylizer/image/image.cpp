#include "api.hpp"

#include "memory_image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.hpp"

namespace stylizer { inline namespace images {
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

	stylizer::dynamic_memory_image<stdmath::vector<uint8_t, 4>> load_stb_image(context&, std::span<std::byte> memory, std::string_view extension /* = {} */) {
		int x, y, n;
		auto data = stbi_load_from_memory((uint8_t*)memory.data(), memory.size(), &x, &y, &n, 4);
		std::span<stdmath::vector<uint8_t, 4>> span; std::vector<stdmath::vector<uint8_t, 4>> tmp;
		if(n == 4)
			span = {(stdmath::vector<uint8_t, 4>*)data, static_cast<size_t>(x * y)};
		else if(n == 3) {
			std::span<stdmath::vector<uint8_t, 3>> span3 = {(stdmath::vector<uint8_t, 3>*)data, static_cast<size_t>(x * y)};
			tmp.reserve(span3.size());
			for(auto& vec: span3)
				tmp.emplace_back(vec, 1); // If there is no alpha data we set alpha to 1
			span = tmp;
		}

		return {span, stylizer::dynamic_memory_image<uint8_t>::extents_t(x, y, 1)};
	}

	stylizer::maybe_owned<stylizer::image> load_stb_image_generic(context& ctx, std::span<std::byte> memory, std::string_view extension /* = {} */) {
		auto out = load_stb_image(ctx, memory, extension);
		return stylizer::maybe_owned<stylizer::dynamic_memory_image<stdmath::vector<uint8_t, 4>>>::make_owned_and_move(out).move_as<stylizer::image>();
	}
}}