#pragma once

#include <cstdarg>
#include <memory>

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
	constexpr error_code(error_domain *domain, int code): _domain(domain), _code(code) {}
	error_code(const error_code&) = default;
	error_code &operator=(const error_code &r) = default;
	bool operator==(const error_code &r) const { return _code == r._code && _domain == r._domain; }
	bool operator!=(const error_code &r) const { return _code != r._code || _domain != r._domain; }
	explicit operator bool() const { return _code != 0; }
};

extern error_code generic_error_code;

//============================================================================
// error_data
//============================================================================

class error_data {
public:
	virtual ~error_data();
	virtual void destroy();
	virtual const char *what() const;
};

class static_error_data : public error_data {
public:
	virtual void destroy() override;
};

struct _error_data_deleter {
	void operator()(error_data *ed) {
		ed->destroy();
	}
};

using error_data_uptr = std::unique_ptr<error_data, _error_data_deleter>;

//============================================================================
// error
//============================================================================

enum class error_verbosity {
	quiet,
	verbose,
	extra,
};

class error {
protected:
	error_verbosity _verbosity = error_verbosity::verbose;
	error_code _code;
	error_data_uptr _data;
	std::unique_ptr<char[]> _message;

public:
	error() = default;
	explicit error(error_verbosity v): _verbosity(v) {}
	error(error&&) = default;
	error(const error&) = delete;
	virtual ~error();

	error &operator=(error&&) = default;
	error &operator=(const error&) = delete;

	void set(error_code code = generic_error_code);
	void set(const char *format, ...);
	void set(error_code code, const char *format, ...);

	virtual void set_va(error_code code, const char *format, va_list va);
	virtual void set_data(error_code code, error_data_uptr data);

	const char *what() const;
	error_code code() const { return _code; }
	error_verbosity verbosity() const { return _verbosity; }
	error_data *data() const { return _data.get(); }

	explicit operator bool() const { return static_cast<bool>(_code); }
};

//============================================================================
// abort_error
//============================================================================

class abort_error : public error {
public:
	void set_va(error_code code, const char *format, va_list va) override;
	void set_data(error_code code, error_data_uptr data) override;
};

extern abort_error default_error;

} // namespace zbs
