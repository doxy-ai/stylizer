#ifndef STYLIZER_INTERFACES_IS_AVAILABLE
#define STYLIZER_INTERFACES_IS_AVAILABLE

#ifdef __cplusplus
	#include "config.hpp"
#else
	#include "config.h"
#endif

#include "interfaces.manageable.h"

//////////////////////////////////////////////////////////////////////
// # Interfaces
//////////////////////////////////////////////////////////////////////

STYLIZER_BEGIN_NAMESPACE

	struct stylizer_api_device;
	struct stylizer_api_surface;

	// Context
	typedef struct {
		stylizer_api_device* device;
		stylizer_api_surface* surface;
	} STYLIZER_PREFIXED_SWITCH_C_OR_CPP(context, context_c);

STYLIZER_END_NAMESPACE

#endif // STYLIZER_INTERFACES_IS_AVAILABLE