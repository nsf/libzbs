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

string operator+(const string &lhs, slice<const char> rhs) {
	string out;
	out.reserve(lhs.len() + rhs.len());
	out.append(lhs);
	out.append(rhs);
	return out;
}

string operator+(slice<const char> lhs, const string &rhs) {
	string out;
	out.reserve(lhs.len() + rhs.len());
	out.append(lhs);
	out.append(rhs);
	return out;
}

string operator+(string &&lhs, slice<const char> rhs) {
	lhs.append(rhs);
	return lhs;
}

string operator+(slice<const char> lhs, string &&rhs) {
	rhs.insert(0, lhs);
	return rhs;
}

} // namespace zbs
