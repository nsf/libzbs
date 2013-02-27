#pragma once

namespace zbs {

//============================================================================
// error_domain
//============================================================================

struct error_domain {};

//============================================================================
// error_code
//============================================================================

class error_code {
	error_domain *_domain;
	int _code;

public:
	constexpr error_code(): _domain(nullptr), _code(0) {}
	constexpr error_code(error_domain &domain, int code): _domain(&domain), _code(code) {}
	constexpr error_code(const error_code &r): _domain(r._domain), _code(r._code) {}
	error_code &operator=(const error_code &r) = default;
	bool operator==(const error_code &r) const { return _code == r._code && _domain == r._domain; }
	bool operator!=(const error_code &r) const { return _code != r._code || _domain != r._domain; }
	explicit operator bool() const { return _code != 0; }
};

extern error_code generic_error_code;

//============================================================================
// error
//
// It has no virtual destructor, because you shouldn't store derived errors
// using a base class pointer.
//============================================================================

class error {
protected:
	error_code _code;

public:
	error_code code() const;
	virtual void set(error_code code, const char *format, ...);
	virtual const char *what() const;
	explicit operator bool() const;
};

//============================================================================
// verbose_error
//============================================================================

class verbose_error : public error {
protected:
	char *_message = nullptr;

public:
	~verbose_error();
	void set(error_code code, const char *format, ...) override;
	const char *what() const override;
};

//============================================================================
// abort_error
//============================================================================

class abort_error_t : public error {
public:
	void set(error_code code, const char *format, ...) override;
};

extern abort_error_t abort_error;

} // namespace zbs
