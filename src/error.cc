#include "zbs/_error.hh"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include "zbs/_utils.hh"

namespace zbs {

static std::unique_ptr<char[]> _vasprintf(const char *format, va_list va) {
	va_list va2;
	va_copy(va2, va);

	int n = std::vsnprintf(nullptr, 0, format, va2);
	va_end(va2);
	if (n < 0) {
		std::fprintf(stderr, "null vsnprintf error\n");
		std::abort();
	}
	if (n == 0) {
		char *buf = new (or_die) char[1];
		buf[0] = '\0';
		return std::unique_ptr<char[]>{buf};
	}

	char *buf = new (or_die) char[n+1];
	int nw = std::vsnprintf(buf, n+1, format, va);
	if (n != nw) {
		std::fprintf(stderr, "vsnprintf failed to write n bytes\n");
		std::abort();
	}

	return std::unique_ptr<char[]>{buf};
}

//============================================================================
// error_domain
//============================================================================

error_domain generic_error_domain;

//============================================================================
// error_code
//============================================================================

error_code generic_error_code(&generic_error_domain, 1);

//============================================================================
// error_data
//============================================================================

error_data::~error_data() {}
void error_data::destroy() { delete this; }
const char *error_data::what() const { return ""; }
void static_error_data::destroy() {}

//============================================================================
// error
//============================================================================

error::~error() {}

void error::set(error_code code) {
	set_data(code, nullptr);
}

void error::set(const char *format, ...) {
	va_list va;
	va_start(va, format);
	set_va(generic_error_code, format, va);
	va_end(va);
}

void error::set(error_code code, const char *format, ...) {
	va_list va;
	va_start(va, format);
	set_va(code, format, va);
	va_end(va);
}

void error::set_va(error_code code, const char *format, va_list va) {
	_code = code;
	_data.reset();
	_message.reset();

	if (_verbosity > error_verbosity::quiet) {
		_message = _vasprintf(format, va);
	}
}

void error::set_data(error_code code, error_data_uptr data) {
	_code = code;
	_data.reset();
	_message.reset();

	if (_verbosity > error_verbosity::quiet) {
		_data = std::move(data);
	}
}

const char *error::what() const {
	if (_data)
		return _data->what();
	if (_message)
		return _message.get();
	return "";
}

//============================================================================
// abort_error
//============================================================================

void abort_error::set_va(error_code, const char *format, va_list va) {
	std::vfprintf(stderr, format, va);
	std::fprintf(stderr, "\n");
	abort();
}

void abort_error::set_data(error_code, error_data_uptr data) {
	if (data)
		std::fprintf(stderr, "%s\n", data->what());
	abort();
}

abort_error default_error;

} // namespace zbs
