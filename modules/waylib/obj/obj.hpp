#pragma once

#include "waylib/core/core.hpp"

WAYLIB_BEGIN_NAMESPACE

	extern "C" {
		#include "obj.h"
	}

	namespace obj {
		result<model> load(const std::filesystem::path& file_path);
	}

WAYLIB_END_NAMESPACE