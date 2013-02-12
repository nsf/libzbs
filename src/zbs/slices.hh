#pragma once

#include <utility>

#include "_utils.hh"
#include "slice.hh"

namespace zbs {
namespace slices {

/// Finds the subslice `subs` within the slice `s`.
///
/// Template parameters must have the same type disregarding `const` qualifier.
/// The function has O(N) complexity.
///
/// @retval N The index of the first occurrence of the `subs`.
/// @retval -1 The `sep` was not found within `s`.
template <typename T, typename U>
int index(slice<T> s, slice<U> subs) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	const int n = subs.len();
	if (n == 0) {
		return 0;
	}
	const U c = subs[0];
	if (n == 1) {
		for (int i = 0; i < s.len(); i++) {
			if (s[i] == c) {
				return i;
			}
		}
		return -1;
	}
	for (int i = 0; i+n <= s.len(); i++) {
		if (s[i] == c && s.sub(i, i+n) == subs) {
			return i;
		}
	}
	return -1;
}

/// Checks if the slice `s` contains the subslice `subs`.
///
/// Template parameters must have the same type disregarding `const` qualifier.
/// The function has O(N) complexity.
template <typename T, typename U>
bool contains(slice<T> s, slice<U> subs) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	return index(s, subs) >= 0;
}

/// Reverses the order of the elements in the slice `s`.
///
/// The function has O(N) complexity.
template <typename T>
void reverse(slice<T> s) {
	const int lenm1 = s.len() - 1;
	const int mid = s.len() / 2;
	for (int i = 0; i < mid; i++) {
		std::swap(s[i], s[lenm1-i]);
	}
}

}} // namespace zbs::slices
