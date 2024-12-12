#pragma once

#ifdef STYLIZER_USE_ABSTRACT_API
	#include <stylizer/api/api.hpp>
	#define STYLIZER_API_NAMESPACE stylizer::api
	#define STYLIZER_API_TYPE(type) STYLIZER_API_NAMESPACE::type*
#else // STYLIZER_USE_ABSTRACT_API
	#include <stylizer/api/current_backend.hpp>
	#define STYLIZER_API_NAMESPACE stylizer::api::current_backend
	#define STYLIZER_API_TYPE(type) STYLIZER_API_NAMESPACE::type
#endif // STYLIZER_USE_ABSTRACT_API

#define STYLIZER_ENABLE_DEFAULT_PARAMETERS // We have default parameters in C++
#define STYLIZER_DEFAULT_PARAMETER(param) = param

#define STYLIZER_BEGIN_NAMESPACE namespace stylizer {
#define STYLIZER_END_NAMESPACE }

#include <string>

namespace stylizer {

	extern "C" {
		#include "config.h"
	}

} // namespace stylizer