#pragma once

#include <stylizer/core/core.hpp>

#include <filesystem>

namespace stylizer::obj {
	core::model load(const std::filesystem::path& file_path, context& ctx = context::null);
}