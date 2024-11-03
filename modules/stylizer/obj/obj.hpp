#pragma once

#include "stylizer/core/core.hpp"

STYLIZER_BEGIN_NAMESPACE

	extern "C" {
		#include "obj.h"
	}

	namespace obj {
		model load(const std::filesystem::path& file_path);
	}

STYLIZER_END_NAMESPACE