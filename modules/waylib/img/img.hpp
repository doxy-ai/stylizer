#pragma once

#include "waylib/core/core.hpp"

WAYLIB_BEGIN_NAMESPACE

	extern "C" {
		#include "img.h"
	}

	namespace img {
		image load(const std::filesystem::path& file_path);
		image load_from_memory(std::span<std::byte> data);
	}

WAYLIB_END_NAMESPACE