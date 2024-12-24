#pragma once

#include <stylizer/core/core.hpp>

#include <filesystem>

namespace stylizer::img {

	core::image load(const std::filesystem::path& file_path);
	core::image load_from_memory(std::span<const std::byte> data);

	inline core::image load_frames(std::span<const std::filesystem::path> paths) {
		std::vector<auto_release<core::image>> images; images.reserve(paths.size());
		for(auto& path: paths)
			images.emplace_back(load(path));

		return core::image::merge({(core::image*)images.data(), images.size()});
	}

} // namespace stylizer::img