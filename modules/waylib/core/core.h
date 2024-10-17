#ifndef WAYLIB_CORE_IS_AVAILABLE
#define WAYLIB_CORE_IS_AVAILABLE

#ifndef __cplusplus
	#include "config.h"
	#include "optional.h"

	#include <webgpu/webgpu.h>
#endif

#include "interfaces.h"

//////////////////////////////////////////////////////////////////////
// # Errors
//////////////////////////////////////////////////////////////////////

WAYLIB_NULLABLE(const char*) get_error_message();
void clear_error_message();

//////////////////////////////////////////////////////////////////////
// # Thread Pool
//////////////////////////////////////////////////////////////////////

struct WAYLIB_PREFIXED(thread_pool_future);

WAYLIB_NULLABLE(WAYLIB_PREFIXED(thread_pool_future)*) WAYLIB_PREFIXED(thread_pool_enqueue)(
	void(*function)(),
	bool return_future
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	, WAYLIB_OPTIONAL(size_t) initial_pool_size
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
);

void WAYLIB_PREFIXED(release_thread_pool_future)(
	WAYLIB_PREFIXED(thread_pool_future)* future
);

void WAYLIB_PREFIXED(thread_pool_future_wait)(
	WAYLIB_PREFIXED(thread_pool_future)* future,
	WAYLIB_OPTIONAL(float) seconds_until_timeout
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
);

#endif // WAYLIB_CORE_IS_AVAILABLE