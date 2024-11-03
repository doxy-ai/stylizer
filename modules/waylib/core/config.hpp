#pragma once

#if !defined(STYLIZER_NAMESPACE) && !defined(STYLIZER_NO_NAMESPACE)
	#define STYLIZER_NAMESPACE sl
#endif

#ifdef STYLIZER_NO_NAMESPACE
	#define STYLIZER_BEGIN_NAMESPACE
	#define STYLIZER_END_NAMESPACE
	#undef STYLIZER_NAMESPACE
	#define STYLIZER_NAMESPACE
#else
	#define STYLIZER_BEGIN_NAMESPACE namespace STYLIZER_NAMESPACE {
	#define STYLIZER_END_NAMESPACE }
#endif


#define STYLIZER_ENABLE_DEFAULT_PARAMETERS // We have default parameters in C++

#if defined(__cpp_lib_expected) && __cpp_lib_expected > 202202L
	#include <expected>
#else // std::expected?
	#include <tl/expected.hpp>
#endif // std::expected?

#include <string>


STYLIZER_BEGIN_NAMESPACE

#if defined(__cpp_lib_expected) && __cpp_lib_expected > 202202L
	using std::expected;
	using std::unexpected;
#else // std::expected?
	using tl::expected;
	using tl::unexpected;
#endif // std::expected?

	extern "C" {
		#include "config.h"
	}

STYLIZER_END_NAMESPACE