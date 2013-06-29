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

} // namespace zbs
