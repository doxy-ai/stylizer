#include <variant>

namespace stylizer {
	template<typename T>
	struct maybe_owned_t: public std::variant<T, T*> {
		using super = std::variant<T, T*>;
		using super::super;
		// maybe_owned_t() : super(nullptr) {}
		// maybe_owned_t(T* ptr) : super(ptr) {}
		// maybe_owned_t(T&& ref) : super(std::move(ref)) {}
		// maybe_owned_t(const maybe_owned_t&) = default;
		// maybe_owned_t(maybe_owned_t&&) = default;
		// maybe_owned_t& operator=(const maybe_owned_t&) = default;
		// maybe_owned_t& operator=(maybe_owned_t&&) = default;

		T& get() {
			if(this->index() == 0)
				return std::get<0>(*this);

			auto ptr = std::get<1>(*this);
			assert(ptr);
			return *ptr;
		}
		const T& get() const {
			if(this->index() == 0)
				return std::get<0>(*this);

			auto ptr = std::get<1>(*this);
			assert(ptr);
			return *ptr;
		}

		inline T* operator->() { return &get(); }
		inline const T* operator->() const { return *get(); }
		inline T& operator*() { return get(); }
		inline const T& operator*() const { return get(); }

		inline bool owned() const { return this->index() == 0; }

		bool valid() const {
			return (this->index() == 0 && std::get<0>(*this))
				|| (this->index() == 1 && std::get<1>(*this) && *std::get<1>(*this));
		}
	};
}