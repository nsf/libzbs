#include "zbs/_slice.hh"

namespace zbs {

#define _slice_const_char_operator(op)					\
bool operator op(slice<const char> lhs, slice<const char> rhs) {	\
	return operator op<const char, const char>(lhs, rhs);		\
}

_slice_const_char_operator(==)
_slice_const_char_operator(<)
_slice_const_char_operator(!=)
_slice_const_char_operator(>=)
_slice_const_char_operator(<=)
_slice_const_char_operator(>)

#undef _slice_const_char_operator

int hash<slice<const byte>>::operator()(slice<const byte> s, int seed) {
	constexpr unsigned M0 = 2860486313U;
	constexpr unsigned M1 = 3267000013U;
	unsigned hash = M0 ^ seed;
	for (const auto &b : s)
		hash = (hash ^ b) * M1;
	return hash;
}

} // namespace zbs
