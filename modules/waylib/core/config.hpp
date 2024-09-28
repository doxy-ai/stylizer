#pragma once

#if !defined(WAYLIB_NAMESPACE) && !defined(WAYLIB_NO_NAMESPACE)
	#define WAYLIB_NAMESPACE wl
#endif

#ifdef WAYLIB_NO_NAMESPACE
	#define WAYLIB_BEGIN_NAMESPACE
	#define WAYLIB_END_NAMESPACE
	#undef WAYLIB_NAMESPACE
	#define WAYLIB_NAMESPACE
#else
	#define WAYLIB_BEGIN_NAMESPACE namespace WAYLIB_NAMESPACE {
	#define WAYLIB_END_NAMESPACE }
#endif


#define WAYLIB_ENABLE_DEFAULT_PARAMETERS // We have default parameters in C++

#if defined(__cpp_lib_expected) && __cpp_lib_expected > 202202L
	#include <expected>
#else // std::expected?
	#include <tl/expected.hpp>
#endif // std::expected?

#include <string>


WAYLIB_BEGIN_NAMESPACE

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

WAYLIB_END_NAMESPACE