#pragma once

#include <cstddef>
#include <filesystem>
#include <unordered_map>
#include "../thirdparty/mio.hpp"

namespace stylizer {
	std::unordered_map<std::filesystem::path, mio::mmap_source>& get_loaded_files();
	std::span<std::byte> load_file(const std::filesystem::path& file);

	template<typename Tfunc>
	auto load_file(const std::filesystem::path& file, const Tfunc& func) {
		return func(load_file(file), file.extension().string());
	}

	template<typename Tfunc>
	auto load_file(struct context& ctx, const std::filesystem::path& file, const Tfunc& func) {
		return func(ctx, load_file(file), file.extension().string());
	}
}