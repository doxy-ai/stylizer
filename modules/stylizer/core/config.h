#ifndef STYLIZER_CONFIG_16A03E22F522119CEA5F6E1BCC86456D
#define STYLIZER_CONFIG_16A03E22F522119CEA5F6E1BCC86456D

#ifndef STYLIZER_BEGIN_NAMESPACE
	#define STYLIZER_BEGIN_NAMESPACE
	#define STYLIZER_END_NAMESPACE
#endif
#ifndef STYLIZER_DEFAULT_PARAMETER
	#define STYLIZER_DEFAULT_PARAMETER(param)
#endif

#ifdef __cplusplus
	#define STYLIZER_SWITCH_C_OR_CPP(Ctype, CPPtype) CPPtype
#else
	#define STYLIZER_SWITCH_C_OR_CPP(Ctype, CPPtype) Ctype
#endif

#define STYLIZER_ENUM STYLIZER_SWITCH_C_OR_CPP(enum, enum class)
#define C_PREPEND(pre, base) STYLIZER_SWITCH_C_OR_CPP(pre##base, base)

#ifdef STYLIZER_C_PREFIX
	#define STYLIZER_PREFIXED(name) STYLIZER_C_PREFIX ## name
#else
	#define STYLIZER_PREFIXED(name) name
#endif
#define STYLIZER_PREFIXED_SWITCH_C_OR_CPP(Ctype, CPPtype) STYLIZER_SWITCH_C_OR_CPP(STYLIZER_PREFIXED(Ctype), CPPtype)
#define STYLIZER_PREFIXED_SWITCH_C_OR_CPP_SINGLE(type) STYLIZER_PREFIXED_C_CPP_TYPE(type, type)

#endif // STYLIZER_CONFIG_16A03E22F522119CEA5F6E1BCC86456D