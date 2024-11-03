#pragma once

#include "waylib/core/core.hpp"

STYLIZER_BEGIN_NAMESPACE

	extern "C" {
		#include "img.h"
	}

	namespace img {
		image load(const std::filesystem::path& file_path);
		image load_from_memory(std::span<std::byte> data);

		inline image load_frames(std::span<const std::filesystem::path> paths) {
			std::vector<auto_release<image>> images; images.reserve(paths.size());
			for(auto& path: paths)
				images.emplace_back(load(path));

			return image::merge({(image*)images.data(), images.size()});
		}
	}

STYLIZER_END_NAMESPACE