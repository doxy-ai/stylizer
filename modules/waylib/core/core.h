#ifndef STYLIZER_CORE_IS_AVAILABLE
#define STYLIZER_CORE_IS_AVAILABLE

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
	#define STYLIZER_MAIN_LOOP_CONTINUE return
	#define STYLIZER_MAIN_LOOP_BREAK emscripten_cancel_main_loop()
	#define STYLIZER_MAIN_LOOP(continue_expression, body)\
		auto callback = [&]() {\
			if(!(continue_expression)) STYLIZER_MAIN_LOOP_BREAK;\
			body\
		};\
		emscripten_set_main_loop(STYLIZER_NAMESPACE::closure2function_pointer(callback), 0, true);
#else // !__EMSCRIPTEN__
	#define STYLIZER_MAIN_LOOP(continue_expression, body)\
		while(continue_expression) {\
			body\
		}
	#define STYLIZER_MAIN_LOOP_CONTINUE continue
	#define STYLIZER_MAIN_LOOP_BREAK break
#endif // __EMSCRIPTEN__


//////////////////////////////////////////////////////////////////////
// # Errors
//////////////////////////////////////////////////////////////////////


STYLIZER_NULLABLE(const char*) get_error_message();
void clear_error_message();


//////////////////////////////////////////////////////////////////////
// # Thread Pool
//////////////////////////////////////////////////////////////////////


struct STYLIZER_PREFIXED(thread_pool_future);

STYLIZER_NULLABLE(STYLIZER_PREFIXED(thread_pool_future)*) STYLIZER_PREFIXED(thread_pool_enqueue)(
	void(*function)(),
	bool return_future
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	, STYLIZER_OPTIONAL(size_t) initial_pool_size
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
);

void STYLIZER_PREFIXED(release_thread_pool_future)(
	STYLIZER_PREFIXED(thread_pool_future)* future
);

void STYLIZER_PREFIXED(thread_pool_future_wait)(
	STYLIZER_PREFIXED(thread_pool_future)* future,
	STYLIZER_OPTIONAL(float) seconds_until_timeout
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
);


//////////////////////////////////////////////////////////////////////
// # wgpu_state
//////////////////////////////////////////////////////////////////////


// STYLIZER_OPTIONAL(STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)) STYLIZER_PREFIXED(default_state_from_instance)(
// 	WGPUInstance instance,
// 	STYLIZER_NULLABLE(WGPUSurface) surface
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= nullptr
// #endif
// 	, bool prefer_low_power
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= false
// #endif
// );

// STYLIZER_OPTIONAL(STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)) STYLIZER_PREFIXED(create_state)(
// 	STYLIZER_NULLABLE(WGPUSurface) surface
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= nullptr
// #endif
// 	, bool prefer_low_power
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= false
// #endif
// );

// void STYLIZER_PREFIXED(release_state)(
// 	STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
// 	bool adapter_release
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= true
// #endif
// 	, bool instance_release
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= true
// #endif
// );

// STYLIZER_PREFIXED(surface_configuration) STYLIZER_PREFIXED(default_surface_configuration)();

// void STYLIZER_PREFIXED(state_configure_surface)(
// 	STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
// 	STYLIZER_PREFIXED_C_CPP_TYPE(vec2u, vec2uC) size,
// 	STYLIZER_PREFIXED(surface_configuration) config
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= {}
// #endif
// );

// STYLIZER_OPTIONAL(STYLIZER_PREFIXED_C_CPP_TYPE(texture, textureC)) STYLIZER_PREFIXED(current_surface_texture)(
// 	STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state
// );

// STYLIZER_OPTIONAL(STYLIZER_PREFIXED_C_CPP_TYPE(drawing_state, drawing_stateC)) STYLIZER_PREFIXED(begin_drawing_to_surface)(
// 	STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
// 	STYLIZER_OPTIONAL(colorC) clear_color
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= {}
// #endif
// );

#endif // STYLIZER_CORE_IS_AVAILABLE