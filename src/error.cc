#include "zbs/_error.hh"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include "zbs/fmt.hh"

namespace zbs {

//============================================================================
// error
//============================================================================

error_code error::code() const {
	return _code;
}

void error::set(error_code code, const char*, ...) {
	_code = code;
}

const char *error::what() const {
	return "";
}

error::operator bool() const {
	return (bool)_code;
}

static error_domain generic_error_domain;
error_code generic_error_code(generic_error_domain, 1);

//============================================================================
// verbose_error
//============================================================================

verbose_error::~verbose_error() {
	if (_message) {
		::free(_message);
	}
}

void verbose_error::set(error_code code, const char *format, ...) {
	va_list vl;
	_code = code;

	va_start(vl, format);
	auto s = fmt::vsprintf(format, vl);
	va_end(vl);
	_message = s.detach_unsafe();
}

const char *verbose_error::what() const {
	if (_message) {
		return _message;
	}
	return "";
}

//============================================================================
// abort_error
//============================================================================

void abort_error::set(error_code, const char *format, ...) {
	fprintf(stderr, "abort: ");
	va_list vl;
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fprintf(stderr, "\n");
	abort();
}

abort_error default_error;

} // namespace zbs
