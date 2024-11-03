#ifndef STYLIZER_ALLOCATED_6876E453E9AB8EC1A18EDA3CC811F7AF
#define STYLIZER_ALLOCATED_6876E453E9AB8EC1A18EDA3CC811F7AF

#ifdef __cplusplus
	template<typename T>
	struct managable {
		bool is_managed = false;
		T value;

		managable() : is_managed(false) {}
		managable(const T& value) : is_managed(false), value(value) {}
		managable(T&& value) : is_managed(false), value(std::move(value)) {}
		managable(bool has_value, const T& value) : is_managed(has_value), value(value) {}
		managable(const managable&) = default;
		managable(managable&&) = default;
		managable& operator=(const managable&) = default;
		managable& operator=(managable&&) = default;

		operator bool() const noexcept { return is_managed; }
		T& operator*() noexcept{ return value; }
		const T& operator*() const noexcept { return value; }
		T* operator->() noexcept { return &value; }
		const T* operator->() const noexcept { return &value; }

		auto& operator[](size_t i) noexcept { return value[i]; }
		const auto& operator[](size_t i) const noexcept { return value[i]; }
	};
	#ifdef STYLIZER_NAMESPACE_NAME
		#define STYLIZER_MANAGEABLE(type) STYLIZER_NAMESPACE_NAME::managable<type>
	#else
		#define STYLIZER_MANAGEABLE(type) managable<type>
	#endif
#else
	#define STYLIZER_MANAGEABLE(type) struct {\
		bool is_managed;\
		type value;\
	}
#endif

#endif // STYLIZER_ALLOCATED_6876E453E9AB8EC1A18EDA3CC811F7AF