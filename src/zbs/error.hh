#pragma once

#include "public_config.h"

//============================================================================
// a couple of useful error checking macros
//============================================================================
#ifdef ZBS_ENABLE_ASSERT
	#define _ZBS_ASSERT(expr)					\
	do {								\
		if (!(expr)) {						\
			zbs::_assert_abort(#expr, __FILE__, __LINE__,	\
				__PRETTY_FUNCTION__);			\
		}							\
	} while (0)
#else
	#define _ZBS_ASSERT(expr) ((void)0)
#endif

#define _ZBS_BOUNDS_CHECK(index, length) \
	_ZBS_ASSERT((unsigned int)(index) < (unsigned int)(length))

namespace zbs {

void _assert_abort(const char *assertion, const char *file, int line, const char *func);

struct error_domain {};

class error_code {
	error_domain *_domain;
	int _code;

public:
	constexpr error_code(error_domain &domain, int code): _domain(&domain), _code(code) {}
	constexpr error_code(const error_code &r): _domain(r._domain), _code(r._code) {}
	error_code &operator=(const error_code &r) = default;
	bool operator==(const error_code &r) const { return _code == r._code && _domain == r._domain; }
	bool operator!=(const error_code &r) const { return _code != r._code || _domain != r._domain; }
	explicit operator bool() const { return _code != 0; }
};

// It has no virtual destructor, because you shouldn't store derived errors
// using a base class pointer.
class error {
	error_code _code;

public:
	error_code code() const;
	virtual void set(error_code code, const char *format, ...);
	virtual const char *what() const;
	explicit operator bool() const;
};

class string_error : public error {
	const char *_message;

public:
	string_error();
	~string_error();
	void set(error_code code, const char *format, ...) override;
	const char *what() const override;
};

} // namespace zbs
