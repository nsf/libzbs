#include "zbs/string.hh"

namespace zbs {

template class basic_string<char>;
template class basic_string<char16_t>;
template class basic_string<char32_t>;

//============================================================================
// utf-8 string
//============================================================================

string::string(slice<const char> r): basic_string<char>(r) {}
string::string(const char *cstr): string(slice<const char>(cstr)) {}

string &string::operator=(slice<const char> r) {
	basic_string<char>::operator=(r);
	return *this;
}

string &string::operator=(const char *cstr) {
	return operator=(slice<const char>(cstr));
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
