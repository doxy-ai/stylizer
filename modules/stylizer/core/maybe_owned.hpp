#include <variant>

namespace stylizer {
	template<typename T>
	struct maybe_owned_t: public std::variant<T, T*> {
		using super = std::variant<T, T*>;
		using super::super;

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