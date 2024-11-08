#ifndef STYLIZER_CONFIG_16A03E22F522119CEA5F6E1BCC86456D
#define STYLIZER_CONFIG_16A03E22F522119CEA5F6E1BCC86456D

#ifdef __cplusplus
	#define STYLIZER_C_OR_CPP_TYPE(Ctype, CPPtype) CPPtype
#else
	#define STYLIZER_C_OR_CPP_TYPE(Ctype, CPPtype) Ctype
#endif

#define STYLIZER_ENUM STYLIZER_C_OR_CPP_TYPE(enum, enum class)
#define C_PREPEND(pre, base) STYLIZER_C_OR_CPP_TYPE(pre##base, base)

#ifdef STYLIZER_C_PREFIX
	#define STYLIZER_PREFIXED(name) STYLIZER_C_PREFIX ## name
#else
	#define STYLIZER_PREFIXED(name) name
#endif
#define STYLIZER_PREFIXED_C_CPP_TYPE(Ctype, CPPtype) STYLIZER_C_OR_CPP_TYPE(STYLIZER_PREFIXED(Ctype), CPPtype)
#define STYLIZER_PREFIXED_C_CPP_TYPE_SINGLE(type) STYLIZER_PREFIXED_C_CPP_TYPE(type, type)

#ifdef __EMSCRIPTEN__
	#define EMSCRIPTEN_FLAGS(type) type ## Flags
#else
	#define EMSCRIPTEN_FLAGS(type) type
#endif

#endif // STYLIZER_CONFIG_16A03E22F522119CEA5F6E1BCC86456D