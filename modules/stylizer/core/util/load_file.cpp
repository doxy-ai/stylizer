#include "load_file.hpp"
#include "../api.hpp"
#include <filesystem>

std::unordered_map<std::filesystem::path, mio::mmap_source>& stylizer::get_loaded_files() {
	static std::unordered_map<std::filesystem::path, mio::mmap_source> loaded_files;
	return loaded_files;
}

std::span<std::byte> stylizer::load_file(const std::filesystem::path& f) {
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