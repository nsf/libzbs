// This file may contain portions of the code derived/adopted from the Go
// standard library, which is covered by a BSD-style license. You can find more
// details in 3rdparty/go_license.txt file.

#pragma once

#include <algorithm>
#include <utility>

#include "_utils.hh"
#include "_slice.hh"

namespace zbs {
namespace slices {

/// Finds the subslice `subs` within the slice `s`.
///
/// Template parameters must have the same type disregarding `const` qualifier.
/// The function has O(N) complexity.
///
/// @retval N The index of the first occurrence of the `subs`.
/// @retval -1 The `subs` was not found within `s`.
template <typename T, typename U>
int index(slice<T> s, slice<U> subs) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	const int n = subs.len();
	if (n == 0) {
		return 0;
	}
	if (n > s.len()) {
		return -1;
	}
	const T &c = subs[0];
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

/// Finds the first occurrence in the slice `s` of any of the elements in `elems`.
///
/// Template parameters must have the same type disregarding `const` qualifier.
/// The function has O(N) complexity.
///
/// @retval N The index of the first occurrence of any of the elements in `elems`.
/// @retval -1 The `elems` is empty or none of `elems` were found within `s`.
template <typename T, typename U>
int index_any(slice<T> s, slice<U> elems) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	if (elems.len() == 0) {
		return -1;
	}
	for (int i = 0; i < s.len(); i++) {
		const T &a = s[i];
		for (const U &b : elems) {
			if (a == b) {
				return i;
			}
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

/// Returns the number of non-overlapping instances of the slice `sep` in the
/// slice `s`.
///
/// Template parameters must have the same type disregarding `const` qualifier.
template <typename T, typename U>
int count(slice<T> s, slice<U> sep) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	const int n = sep.len();
	if (n == 0) {
		return s.len() + 1;
	}
	if (n > s.len()) {
		return 0;
	}
	int count = 0;
	const T &c = sep[0];
	for (int i = 0; i+n <= s.len(); ) {
		if (s[i] == c && s.sub(i, i+n) == sep) {
			count++;
			i += n;
			continue;
		}
		i++;
	}
	return count;
}

/// Tests if the slice `s` starts with the slice `prefix`.
template <typename T, typename U>
bool starts_with(slice<T> s, slice<U> prefix) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	return s.len() >= prefix.len() && s.sub(0, prefix.len()) == prefix;
}

/// Tests if the slice `s` ends with the slice `suffix`.
template <typename T, typename U>
bool ends_with(slice<T> s, slice<U> suffix) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	return s.len() >= suffix.len() && s.sub(s.len()-suffix.len()) == suffix;
}

/// Finds the last instance of subslice `subs` within the slice `s`.
///
/// Template parameters must have the same type disregarding `const` qualifier.
/// The function has O(N) complexity.
///
/// @retval N The index of the last occurrence of the `subs`.
/// @retval -1 The `subs` was not found within `s`.
template <typename T, typename U>
int last_index(slice<T> s, slice<U> subs) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	int n = subs.len();
	if (n == 0) {
		return s.len();
	}

	const U &c = subs[0];
	for (int i = s.len() - n; i >= 0; i--) {
		if (s[i] == c && (n == 1 || s.sub(i, i+n) == subs)) {
			return i;
		}
	}
	return -1;
}

/// Finds the last occurrence in `s` of any of the `elems`.
///
/// Template parameters must have the same type disregarding `const` qualifier.
/// The function has O(N) complexity.
///
/// @retval N The index of the last occurrence of any of the `elems`.
/// @retval -1 The `elems` is empty or none of `elems` were found within `s`.
template <typename T, typename U>
int last_index_any(slice<T> s, slice<U> elems) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	if (elems.len() == 0) {
		return -1;
	}
	for (int i = s.len()-1; i >= 0; i--) {
		const T &a = s[i];
		for (const U &b : elems) {
			if (a == b) {
				return i;
			}
		}
	}
	return -1;
}

/// Sorts the slice `s`.
template <typename T>
void sort(slice<T> s) {
	std::sort(s.data(), s.data() + s.len());
}

//
// TODO:
// bool             starts_with(slice<T> s, slice<U> prefix)                     // DONE
// bool             ends_with(slice<T> s, slice<U> suffix)                       // DONE
// int              index(slice<T> s, slice<U> subs)                             // DONE
// int              index_any(slice<T> s, slice<U> elems)                        // DONE
// bool             contains(slice<T> s, slice<U> subs)                          // DONE
// void             reverse(slice<T> s)                                          // DONE
// int              count(slice<T> s, slice<U> sep)                              // DONE
// vector<T>        join(slice<slice<T>> ss, slice<U> sep)                       // FAIL
// int              last_index(slice<T> s, slice<U> subs)                        // DONE
// int              last_index_any(slice<T> s, slice<U> elems)                   // DONE
// vector<T>        repeat(slice<T> s, int count)
// vector<T>        replace(slice<T> s, slice<U> old, slice<V> new, int count)
// vector<slice<T>> split(slice<T> s, slice<U> sep)                              // FAIL
// vector<slice<T>> split_n(slice<T> s, slice<U> sep, int n)                     // FAIL
// vector<slice<T>> split_after(slice<T> s, slice<U> sep)                        // FAIL
// vector<slice<T>> split_after_n(slice<T> s, slice<U> sep, int n)               // FAIL
// void             sort(slice<T> s);                                            // DONE

}} // namespace zbs::slices
