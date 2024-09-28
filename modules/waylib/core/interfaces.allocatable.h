#ifndef WAYLIB_ALLOCATED_6876E453E9AB8EC1A18EDA3CC811F7AF
#define WAYLIB_ALLOCATED_6876E453E9AB8EC1A18EDA3CC811F7AF

#ifdef __cplusplus
	template<typename T>
	struct allocatable {
		bool is_allocated = false;
		T value;

		allocatable() : is_allocated(false) {}
		allocatable(const T& value) : is_allocated(false), value(value) {}
		allocatable(T&& value) : is_allocated(false), value(std::move(value)) {}
		allocatable(bool has_value, const T& value) : is_allocated(has_value), value(value) {}
		allocatable(const allocatable&) = default;
		allocatable(allocatable&&) = default;
		allocatable& operator=(const allocatable&) = default;
		allocatable& operator=(allocatable&&) = default;

		operator bool() const noexcept { return is_allocated; }
		T& operator*() noexcept{ return value; }
		const T& operator*() const noexcept { return value; }
		T* operator->() noexcept { return &value; }
		const T* operator->() const noexcept { return &value; }
	};
	#ifdef WAYLIB_NAMESPACE_NAME
		#define WAYLIB_ALLOCATABLE(type) WAYLIB_NAMESPACE_NAME::allocatable<type>
	#else
		#define WAYLIB_ALLOCATABLE(type) allocatable<type>
	#endif
#else
	#define WAYLIB_ALLOCATABLE(type) struct {\
		bool is_allocated;\
		type value;\
	}
#endif

#endif // WAYLIB_ALLOCATED_6876E453E9AB8EC1A18EDA3CC811F7AF