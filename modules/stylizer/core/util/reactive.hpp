#pragma once

#include <reaction/reaction.h>

namespace reaction {
	template<reaction::NonReact T, reaction::IsInvalidation IV = KeepHandle, reaction::IsTrigger TR = ChangeTrig>
	auto& update_if_different(reaction::Var<T, IV, TR>& var, const T& value) {
		auto dbg = var();
		if(dbg != value)
			return var.value(value);
		return var;
	}

	template<reaction::NonReact T, reaction::IsInvalidation IV = KeepHandle, reaction::IsTrigger TR = ChangeTrig>
	auto& update_if_any_different(reaction::Var<T, IV, TR>& var, const T& value) {
		auto dbg = var();
		if(any_of(dbg != value))
			return var.value(value);
		return var;
	}
}