#ifndef WAYLIB_CONFIG_16A03E22F522119CEA5F6E1BCC86456D
#define WAYLIB_CONFIG_16A03E22F522119CEA5F6E1BCC86456D

#ifdef __cplusplus
	#define WAYLIB_C_OR_CPP_TYPE(Ctype, CPPtype) CPPtype
#else
	#define WAYLIB_C_OR_CPP_TYPE(Ctype, CPPtype) Ctype
#endif

#define WAYLIB_ENUM WAYLIB_C_OR_CPP_TYPE(enum, enum class)
#define C_PREPEND(pre, base) WAYLIB_C_OR_CPP_TYPE(pre##base, base)

#ifdef WAYLIB_C_PREFIX
	#define WAYLIB_PREFIXED(name) WAYLIB_C_PREFIX ## name
#else
	#define WAYLIB_PREFIXED(name) name
#endif
#define WAYLIB_PREFIXED_C_CPP_TYPE(Ctype, CPPtype) WAYLIB_C_OR_CPP_TYPE(WAYLIB_PREFIXED(Ctype), CPPtype)
#define WAYLIB_PREFIXED_C_CPP_TYPE_SINGLE(type) WAYLIB_PREFIXED_C_CPP_TYPE(type, type)

#ifdef __EMSCRIPTEN__
	#define EMSCRIPTEN_FLAGS(type) type ## Flags
#else
	#define EMSCRIPTEN_FLAGS(type) type
#endif

#endif // WAYLIB_CONFIG_16A03E22F522119CEA5F6E1BCC86456D