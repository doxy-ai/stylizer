module;

#include "../thirdparty/mio.hpp"

export module stylizer.load_file;

import std.compat;
import stylizer;

namespace stylizer {
	export std::unordered_map<std::filesystem::path, mio::mmap_source>& get_loaded_files() {
		static std::unordered_map<std::filesystem::path, mio::mmap_source> loaded_files;
		return loaded_files;
	}

	export std::span<std::byte> load_file(const std::filesystem::path& f) {
		auto file = std::filesystem::canonical(std::filesystem::absolute(f));

		if(get_loaded_files().contains(file)) {
			auto& mmap = get_loaded_files()[file];
			return {(std::byte*)mmap.data(), mmap.size()};
		}

		std::error_code ec;
		auto& mmap = get_loaded_files()[file] = mio::make_mmap_source(file.string(), ec);
		if(ec) get_error_handler()(stylizer::error_severity::Error, ec.message(), 0);
		return {(std::byte*)mmap.data(), mmap.size()};
	}

	export template<typename Tfunc>
	auto load_file(const std::filesystem::path& file, const Tfunc& func) {
		return func(load_file(file), file.extension().string());
	}

	export template<typename Tfunc>
	auto load_file(context& ctx, const std::filesystem::path& file, const Tfunc& func) {
		return func(ctx, load_file(file), file.extension().string());
	}
}
