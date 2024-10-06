#pragma once
#include "config.hpp"

#include "thirdparty/thread_pool.hpp"
#include "optional.h"

#ifdef __cpp_exceptions
	#include <stdexcept>
#endif

WAYLIB_BEGIN_NAMESPACE

	template<typename T>
	struct auto_release: public T {
		auto_release() : T() {}
		auto_release(T&& raw) : T(std::move(raw)) {}
		auto_release(const auto_release&) = delete;
		auto_release(auto_release&& other) : T(std::move(other)) {}
		auto_release& operator=(T&& raw) { static_cast<T&>(*this) = std::move(raw); return *this; }
		auto_release& operator=(const auto_release&) = delete;
		auto_release& operator=(auto_release&& other) {{ static_cast<T&>(*this) = std::move(static_cast<T&>(other)); return *this; }}

		~auto_release() { if(*this) this->release(); }
	};

	#define WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(type)\
		using type ## C::type ## C;\
		inline type ## C& c() { return *this; }\
		type() {}\
		type(const type&) = default;\
		type& operator=(const type&) = default;\
		type(type ## C&& c) : type ## C(std::move(c)) {}


//////////////////////////////////////////////////////////////////////
// # Error Handling
//////////////////////////////////////////////////////////////////////

	// Forward decls
	template<typename T>
	struct result;
	template<typename T>
	inline T throw_if_error(const result<T>& res);

#ifdef __cpp_exceptions
	struct exception: public std::runtime_error {
		using std::runtime_error::runtime_error;
	};
#endif

	template<typename T>
	struct result: expected<T, std::string> {
		using expected<T, std::string>::expected;

		T throw_if_error() { return WAYLIB_NAMESPACE::throw_if_error(*this); }
	};

	struct void_like{};
	template<>
	struct result<void>: expected<void_like, std::string> {
		using expected<void_like, std::string>::expected;
		static constexpr void_like success = {};

		void throw_if_error() {
			if(!*this)
#ifdef __cpp_exceptions
				throw exception(this->error());
#else
				assert(false, this->error().c_str());
#endif
		}
	};

	namespace detail {
		template<typename T>
		struct void2void_like { using type = T; };
		template<>
		struct void2void_like<void> { using type = void_like; };
		template<typename T>
		using void2void_like_t = void2void_like<T>::type;
	}

	struct errors {
	protected:
		static std::string singleton;
	public:
		inline static void clear() { singleton.clear(); }
		inline static std::string get_and_clear() { return std::move(singleton); }
		inline static WAYLIB_NULLABLE(const char*) get() {
			if(singleton.empty()) return nullptr;
			return singleton.c_str();
		}
		inline static void set_view(const std::string_view view) { singleton = view; }
		inline static void set(const std::string& str) { singleton = str; }

		template<typename T>
		inline static void set(const result<T>& res) {
			if(res) return; // No errors no message
			set(res.error());
		}
	};

#ifdef __cpp_exceptions
	template<typename T>
	inline T throw_if_error(const WAYLIB_OPTIONAL(T)& opt) {
		if(opt.is_allocated) return opt.value;

		auto msg = errors::get_and_clear();
		throw exception(msg.empty() ? "Null value encountered" : msg);
	}
	template<typename T>
	inline T& throw_if_error(const WAYLIB_NULLABLE(T*) opt) {
		if(opt) return *opt;

		auto msg = errors::get_and_clear();
		throw exception(msg.empty() ? "Null value encountered" : msg);
	}
	template<typename T>
	inline T throw_if_error(const result<T>& res) {
		if(res) return *res;
		throw exception(res.error());
	}

	#define WAYLIB_TRY try
	#define WAYLIB_CATCH catch(const std::exception& e) { return unexpected(e.what()); }
#else
	template<typename T>
	inline T throw_if_error(const WAYLIB_OPTIONAL(T)& opt) {
		assert(opt.has_value, errors::get());
		return opt.value;
	}
	template<typename T>
	inline T& throw_if_error(const WAYLIB_NULLABLE(T*) opt) {
		assert(opt, errors::get());
		return *opt;
	}
	template<typename T>
	inline T throw_if_error(const result<T>& res) {
		if(res) return *res;
		throw exception(res.error());
	}

	#define WAYLIB_TRY
	#define WAYLIB_CATCH
#endif


//////////////////////////////////////////////////////////////////////
// # Miscelanious
//////////////////////////////////////////////////////////////////////


	template<typename F, typename... Args>
	auto closure2function_pointer(const F& _f, Args...) {
		static F f = _f;
		return +[](Args... args){ f(std::forward<Args>(args)...); };
	}

	// Creates a c-string from a string view
	// (if the string view doesn't point to a valid cstring a temporary one that
	//		is only valid until the next time this function is called is created)
	template<size_t uniqueID = 0>
	inline const char* cstring_from_view(const std::string_view view) {
		static std::string tmp;
		if(view.data()[view.size()] == '\0') return view.data();
		tmp = view;
		return tmp.c_str();
	}

	template<typename T>
	std::future<T> value2future(T&& value) {
		static std::promise<T> promise;
		promise.set_value(std::move(value));
		return promise.get_future();
	}

	template<typename Tin, typename Tout = Tin>
	inline WAYLIB_OPTIONAL(detail::void2void_like_t<Tout>) res2opt(const result<Tin>& res) {
		if(res) return *res;

		errors::set(res);
		return {};
	}

WAYLIB_END_NAMESPACE