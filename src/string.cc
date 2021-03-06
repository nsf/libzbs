#include "zbs/_string.hh"
#include "zbs/unicode/utf8.hh"

namespace utf8 = zbs::unicode::utf8;

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

// string vs. const char*
string operator+(const string &lhs, const char *rhs) {
	return operator+(lhs, slice<const char>(rhs));
}

string operator+(const char *lhs, const string &rhs) {
	return operator+(slice<const char>(lhs), rhs);
}

string operator+(string &&lhs, const char *rhs) {
	return operator+(lhs, slice<const char>(rhs));
}

string operator+(const char *lhs, string &&rhs) {
	return operator+(slice<const char>(lhs), rhs);
}

// string vs. string
string operator+(const string &lhs, const string &rhs) {
	string out;
	out.reserve(lhs.len() + rhs.len());
	out.append(lhs);
	out.append(rhs);
	return out;
}

string operator+(string &&lhs, string &&rhs) {
	lhs.append(rhs);
	return lhs;
}

string operator+(string &&lhs, const string &rhs) {
	lhs.append(rhs);
	return lhs;
}

string operator+(const string &lhs, string &&rhs) {
	rhs.insert(0, lhs);
	return rhs;
}

// string vs. slice<const char>
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

int hash<string>::operator()(const string &s, int seed) {
	return hash<slice<const char>>()(s, seed);
}

string_iter::string_iter(slice<const char> s): _s(s), _r(0), _offset(0) {
	_r = utf8::decode_rune(_s).rune;
}

string_iter &string_iter::operator++() {
	const int len = utf8::rune_len(_r);
	_s = _s.sub(len);
	_offset += len;
	_r = utf8::decode_rune(_s).rune;
	return *this;
}

bool string_iter::operator==(const string_iter&) const {
	return _s.len() == 0;
}

bool string_iter::operator!=(const string_iter&) const {
	return _s.len() > 0;
}

string_iter begin(const string &s) {
	return string_iter(s);
}

string_iter end(const string&) {
	return string_iter();
}

offset_rune string_iter::operator*() const {
	return {_r, _offset};
}

} // namespace zbs
