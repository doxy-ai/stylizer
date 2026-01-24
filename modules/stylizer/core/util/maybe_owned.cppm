module;

#include <cassert>

export module stylizer.maybe_owned;

import std.compat;

namespace stylizer {
	namespace detail {
		template<typename Base, typename Derived>
		concept base_of = std::derived_from<Derived, Base>;
	}

	export template<typename T>
	struct maybe_owned {\
		bool owned = false;
		T* value;

		static maybe_owned make_owned(T&& moved) { return {new T(std::move(moved)), true}; }
		static maybe_owned make_owned_and_move(T& will_be_moved) { return make_owned(std::move(will_be_moved)); }

		maybe_owned() : value(nullptr), owned(false) {}
		maybe_owned(T& ref, bool owned = false) : value(&ref), owned(owned) {}
		maybe_owned(T&& moved) requires(requires(T a) { {new T(a)}; }) : value(new T(std::move(moved))), owned(true) {}
		maybe_owned(T* ptr, bool owned = false) : value(ptr), owned(owned) {}
		maybe_owned(const maybe_owned& o) requires(requires (T a, T b) { {a = b}; }) { *this = o; }
		maybe_owned(maybe_owned&& o)  { *this = std::move(o); }
		maybe_owned& operator=(const maybe_owned& o) requires(requires (T a, T b) { {a = b}; }) {
			if(owned) release();
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
		maybe_owned& operator=(maybe_owned&& o) {
			if(owned) release();
			value = std::exchange(o.value, nullptr);
			owned = std::exchange(o.owned, false);
			return *this;
		}

		template<detail::base_of<T> Tto>
		maybe_owned<Tto>&& move_as() {
			return std::move((maybe_owned<Tto>&)*this);
		}

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

	export template<typename T>
	std::span<stylizer::maybe_owned<T>> maybe_owned_span(std::span<T> initial) {
		static thread_local std::vector<stylizer::maybe_owned<T>> maybe_owned;
		maybe_owned.clear();
		maybe_owned.reserve(initial.size());
		for(auto& value: initial)
			maybe_owned.push_back(&value);
		return maybe_owned;
	}
}
