#ifndef STYLIZER_OPTIONAL_IS_AVAILABLE
#define STYLIZER_OPTIONAL_IS_AVAILABLE

#ifdef __cplusplus
	namespace detail {
		template<typename T>
		struct is_future : std::false_type {};
		template<typename T>
		struct is_future<std::future<T>> : std::true_type {};
		template<typename T>
		static constexpr bool is_future_v = is_future<T>::value;
	}

	template<typename T>
	struct optional {
		bool has_value = false;
		T value;

		optional() requires(!std::is_reference_v<T> && !detail::is_future_v<T>) : has_value(false) {}
		optional() requires(std::is_reference_v<T>) : has_value(false), value([]() -> T {
			static std::remove_reference_t<T> def = {};
			return def;
		}()) {}
		optional() requires(detail::is_future_v<T>) : has_value(false), value() {}
		optional(const T& value) : has_value(true), value(value) {}
		optional(T&& value) requires(!std::is_reference_v<T>) : has_value(true), value(std::move(value)) {}
		optional(bool has_value, const T& value) : has_value(has_value), value(value) {}
		optional(const optional&) = default;
		optional(optional&&) = default;
		optional& operator=(const optional&) = default;
		optional& operator=(optional&&) = default;

		operator bool() const noexcept { return has_value; }
		T& operator*() noexcept{ return value; }
		const T& operator*() const noexcept { return value; }
		std::remove_reference_t<T>* operator->() noexcept { return &value; }
		const std::remove_reference_t<T>* operator->() const noexcept { return &value; }
	};
	#ifdef STYLIZER_NAMESPACE_NAME
		#define STYLIZER_OPTIONAL(type) STYLIZER_NAMESPACE_NAME::optional<type>
	#else
		#define STYLIZER_OPTIONAL(type) optional<type>
	#endif
#else
	#define STYLIZER_OPTIONAL(type) struct {\
		bool has_value;\
		type value;\
	}
#endif

#define STYLIZER_NULLABLE(type) type

#endif // STYLIZER_OPTIONAL_IS_AVAILABLE