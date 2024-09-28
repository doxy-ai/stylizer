#ifndef WAYLIB_CORE_IS_AVAILABLE
#define WAYLIB_CORE_IS_AVAILABLE

#ifndef __cplusplus
	#include "config.h"
	#include "optional.h"

	#include <webgpu/webgpu.h>
#endif

#include "interfaces.h"

//////////////////////////////////////////////////////////////////////
// # Main Loop Macros
//////////////////////////////////////////////////////////////////////


#ifdef __EMSCRIPTEN__
	#define WAYLIB_MAIN_LOOP_CONTINUE return
	#define WAYLIB_MAIN_LOOP_BREAK emscripten_cancel_main_loop()
	#define WAYLIB_MAIN_LOOP(continue_expression, body)\
		auto callback = [&]() {\
			if(!(continue_expression)) WAYLIB_MAIN_LOOP_BREAK;\
			body\
		};\
		emscripten_set_main_loop(WAYLIB_NAMESPACE::closure_to_function_pointer(callback), 0, true);
#else // !__EMSCRIPTEN__
	#define WAYLIB_MAIN_LOOP(continue_expression, body)\
		while(continue_expression) {\
			body\
		}
	#define WAYLIB_MAIN_LOOP_CONTINUE continue
	#define WAYLIB_MAIN_LOOP_BREAK break
#endif // __EMSCRIPTEN__


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


//////////////////////////////////////////////////////////////////////
// # wgpu_state
//////////////////////////////////////////////////////////////////////


WAYLIB_OPTIONAL(WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)) WAYLIB_PREFIXED(default_state_from_instance)(
	WGPUInstance instance,
	WAYLIB_NULLABLE(WGPUSurface) surface
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	, bool prefer_low_power
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
);

WAYLIB_OPTIONAL(WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)) WAYLIB_PREFIXED(create_state)(
	WAYLIB_NULLABLE(WGPUSurface) surface
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	, bool prefer_low_power
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
);

void WAYLIB_PREFIXED(release_state)(
	WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
	bool adapter_release
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	, bool instance_release
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
);

WAYLIB_PREFIXED(surface_configuration) WAYLIB_PREFIXED(default_surface_configuration)();

void WAYLIB_PREFIXED(state_configure_surface)(
	WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
	WAYLIB_PREFIXED_C_CPP_TYPE(vec2u, vec2uC) size,
	WAYLIB_PREFIXED(surface_configuration) config
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
);


#endif // WAYLIB_CORE_IS_AVAILABLE