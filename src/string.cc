#include "zbs/string.hh"

namespace zbs {

template class basic_string<char>;
template class basic_string<char16_t>;
template class basic_string<char32_t>;

//============================================================================
// utf-8 string
//============================================================================

string::string(const char *cstr) {
	_cap = _len = ::strlen(cstr);
	_data = static_cast<char*>(malloc(_cap + 1));
	::memcpy(_data, cstr, _cap + 1);
}

string &string::operator=(const char *cstr) {
	const int len = ::strlen(cstr);
	_len = 0;
	reserve(len);
	::memcpy(_data, cstr, len + 1);
	_len = len;
	return *this;
}

bool operator==(const string &lhs, const string &rhs) {
	return lhs.len() == rhs.len() && ::strcmp(lhs.c_str(), rhs.c_str()) == 0;
}

bool operator==(const char *lhs, const string &rhs) {
	return ::strcmp(lhs, rhs.c_str()) == 0;
}

bool operator==(const string &lhs, const char *rhs) {
	return ::strcmp(lhs.c_str(), rhs) == 0;
}

} // namespace zbs
