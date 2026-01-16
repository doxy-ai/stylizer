#pragma once

#include <type_traits>
#include <variant>

namespace stylizer {
	template<typename T>
	struct maybe_owned {\
		bool owned = false;
		T* value;

		maybe_owned() : value(nullptr), owned(false) {}
		maybe_owned(T& ref) : value(&ref), owned(false) {}
		maybe_owned(T&& moved) : value(new T(std::move(moved))), owned(true) {}
		maybe_owned(T* ptr) : value(ptr), owned(false) {}
		maybe_owned(const maybe_owned& o) requires(requires (T a, T b) { {a = b}; }) { *this = o; }
		maybe_owned(maybe_owned&& o) = default;
		maybe_owned& operator=(const maybe_owned& o) requires(requires (T a, T b) { {a = b}; }) {
			if(o.owned)	{
				owned = true;
				if constexpr(requires (T t) { { new T(t) }; })
					value = new T(*o);
				else assert(false); // TODO: Can we do better?
			} else {
				owned = false;
				value = o.value;
			}
			return *this;
		}
		maybe_owned& operator=(maybe_owned&&) = default;

		void release() {
			if(owned && value) {
				if constexpr(requires (T t) { {t.release()}; })
					value->release();
				delete value;
			}
			value = nullptr;
		}
		
		inline T& get() { return *value; }
		inline const T& get() const { return *value; }
		inline T& operator*() { return get(); }
		inline const T& operator*() const { return get(); }
		inline T* operator->() { return &get(); }
		inline const T* operator->() const { return &get(); }
	};

	template<typename T>
	std::span<stylizer::maybe_owned<T>> maybe_owned_span(std::span<T> initial) {
		static thread_local std::vector<stylizer::maybe_owned<T>> maybe_owned;
		maybe_owned.clear();
		maybe_owned.reserve(initial.size());
		for(auto& value: initial)
			maybe_owned.push_back(&value);
		return maybe_owned;
	}
}
