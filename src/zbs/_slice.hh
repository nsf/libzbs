#pragma once

#include <cstring>
#include <algorithm>
#include <initializer_list>
#include "_utils.hh"

namespace zbs {


// slice template helper macro, however we unwrap it manually in the `const T`
// slice specialization for documentation purposes, keep that in mind if you're
// editing it
#define _common_slice_part_const(T)						\
private:									\
	const T *_data;								\
	int _len;								\
										\
public:										\
	constexpr slice(): _data(nullptr), _len(0) {}				\
	constexpr slice(std::initializer_list<T> r):				\
		_data(r.begin()), _len(r.size()) {}				\
	template <int N>							\
	constexpr slice(const T (&array)[N]): _data(array), _len(N) {}		\
	constexpr slice(const T *data, int len): _data(data), _len(len) {}	\
	constexpr slice(const slice<T> &r): _data(r.data()), _len(r.len()) {}	\
	constexpr slice(const slice &r) = default;				\
	slice &operator=(const slice &r) = default;				\
	constexpr explicit operator bool() const { return _len != 0; }		\
	constexpr int len() const { return _len; }				\
	constexpr int byte_len() const { return _len * sizeof(T); }		\
	constexpr const T *data() const { return _data; }			\
	slice<const T> sub() const {						\
		return {_data, _len};						\
	}									\
	slice<const T> sub(int begin) const {					\
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);				\
		return {_data + begin, _len - begin};				\
	}									\
	slice<const T> sub(int begin, int end) const {				\
		_ZBS_ASSERT(begin <= end);					\
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);				\
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);				\
		return {_data + begin, end - begin};				\
	}									\
	const T &operator[](int idx) const {					\
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);				\
		return _data[idx];						\
	}									\
private:

template <typename T>
class slice {
	T *_data;
	int _len;

public:

	constexpr slice(): _data(nullptr), _len(0) {}
	template <int N>
	constexpr slice(T (&array)[N]): _data(array), _len(N) {}
	constexpr slice(T *data, int len): _data(data), _len(len) {}
	constexpr slice(const slice &r) = default;
	slice &operator=(const slice &r) = default;
	constexpr explicit operator bool() const { return _len != 0; }
	T &operator[](int idx) {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}
	const T &operator[](int idx) const {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}
	constexpr int len() const { return _len; }
	constexpr int byte_len() const { return _len * sizeof(T); }
	T *data() { return _data; }
	constexpr const T *data() const { return _data; }
	slice sub() {
		return {_data, _len};
	}
	slice sub(int begin) {
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}
	slice sub(int begin, int end) {
		_ZBS_ASSERT(begin <= end);
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);
		return {_data + begin, end - begin};
	}
	slice<const T> sub() const {
		return {_data, _len};
	}
	slice<const T> sub(int begin) const {
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}
	slice<const T> sub(int begin, int end) const {
		_ZBS_ASSERT(begin <= end);
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);
		return {_data + begin, end - begin};
	}
};

template <typename T>
class slice<const T> {
	_common_slice_part_const(T)
};

//============================================================================
// slice template specializations
//
// These are created for implicit conversions from a string literals to the
// slice. We provide specializations for const char[], const char16_t[] and
// const char32_t[] literals ("", u8"", u"", U"").
//
// There was a try to add constexpr magic to the const char* constructor, so
// that the length of string literals is computed at compile-time, however GCC
// failed to figure out that when a constructor is constexpr, maybe it makes
// sense to execute it at compile-time. Clang was doing the right thing.
// However, it was discovered that ::strlen call is optimized away by both
// compilers properly anyway. Therefore adding constexpr isn't worth it.
//============================================================================

/// Slice template specialization for `const char`. It contains everything
/// slice<const T> has, plus a specialized constructor for `const char*` type.
/// @sa slice<const T>
template <>
class slice<const char> {
	static constexpr int _strlen(const char *s) {
		return *s ? (1 + _strlen(s+1)) : 0;
	}
public:
	/// Constructs a slice using the null-terminated C string `str`.
	constexpr slice(const char *str): _data(str), _len(_strlen(str)) {}
	_common_slice_part_const(char)
};

/// Slice template specialization for `const char16_t`. It contains everything
/// slice<const T> has, plus a specialized constructor for `const char16_t*`
/// type.
/// @sa slice<const T>
template <>
class slice<const char16_t> {
	static constexpr int _strlen16(const char16_t *s) {
		return *s ? (1 + _strlen16(s+1)) : 0;
	}
public:
	/// Constructs a slice using the null-terminated UTF-16 C++ string
	/// `str`.
	constexpr slice(const char16_t *str): _data(str), _len(_strlen16(str)) {}
	_common_slice_part_const(char16_t)
};

/// Slice template specialization for `const char32_t`. It contains everything
/// slice<const T> has, plus a specialized constructor for `const char32_t*`
/// type.
/// @sa slice<const T>
template <>
class slice<const char32_t> {
	static constexpr int _strlen32(const char32_t *s) {
		return *s ? (1 + _strlen32(s+1)) : 0;
	}
public:
	/// Constructs a slice using the null-terminated UTF-32 C++ string
	/// `str`.
	constexpr slice(const char32_t *str): _data(str), _len(_strlen32(str)) {}
	_common_slice_part_const(char32_t)
};

#undef _common_slice_part_const

//============================================================================
// slice comparison operators
//============================================================================

template <typename T, typename U>
bool operator==(slice<T> lhs, slice<U> rhs) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	if (lhs.len() != rhs.len()) {
		return false;
	}

	for (int i = 0; i < lhs.len(); i++) {
		if (!(lhs.data()[i] == rhs.data()[i])) {
			return false;
		}
	}
	return true;
}

template <typename T, typename U>
bool operator<(slice<T> lhs, slice<U> rhs) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	for (int i = 0; i < rhs.len(); i++) {
		if (i == lhs.len()) {
			// lhs.len() < rhs.len(), but the common part is ==
			return true;
		}
		if (lhs.data()[i] < rhs.data()[i]) {
			return true;
		}
		if (rhs.data()[i] < lhs.data()[i]) {
			return false;
		}
	}
	return false;
}

template <typename T, typename U>
bool operator!=(slice<T> lhs, slice<U> rhs) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	return !operator==(lhs, rhs);
}

template <typename T, typename U>
bool operator>=(slice<T> lhs, slice<U> rhs) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	return !operator<(lhs, rhs);
}

template <typename T, typename U>
bool operator<=(slice<T> lhs, slice<U> rhs) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	return !operator<(rhs, lhs);
}

template <typename T, typename U>
bool operator>(slice<T> lhs, slice<U> rhs) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	return operator<(rhs, lhs);
}

// additional non-template operator overloads for strings
bool operator==(slice<const char> lhs, slice<const char> rhs);
bool operator<(slice<const char> lhs, slice<const char> rhs);
bool operator!=(slice<const char> lhs, slice<const char> rhs);
bool operator>=(slice<const char> lhs, slice<const char> rhs);
bool operator<=(slice<const char> lhs, slice<const char> rhs);
bool operator>(slice<const char> lhs, slice<const char> rhs);

//============================================================================
// slice comparison operators
//============================================================================

namespace detail {

template <typename T, typename U, bool is_pod>
struct copier;

template <typename T, typename U>
struct copier<T, U, false> {
	static int copy(slice<T> dst, slice<U> src) {
		const int n = std::min(dst.len(), src.len());
		if (n == 0) {
			return 0;
		}

		const T *srcp = src.data();
		T *dstp = dst.data();
		if (srcp == dstp) {
			return n;
		}

		if (srcp < dstp) {
			for (int i = n-1; i >= 0; i--) {
				dstp[i] = srcp[i];
			}
		} else {
			for (int i = 0; i < n; i++) {
				dstp[i] = srcp[i];
			}
		}
		return n;
	}
};

template <typename T, typename U>
struct copier<T, U, true> {
	static int copy(slice<T> dst, slice<U> src) {
		const int n = std::min(dst.len(), src.len());
		if (n == 0) {
			return 0;
		}
		::memmove(dst.data(), src.data(), n * sizeof(T));
		return n;
	}
};

} // namespace zbs::detail

template <typename T, typename U, bool is_pod = std::is_pod<T>::value>
int copy(slice<T> dst, slice<U> src) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	return detail::copier<T, U, is_pod>::copy(dst, src);
}

template <typename T>
constexpr const T *begin(slice<const T> s) { return s.data(); }
template <typename T>
constexpr const T *end(slice<const T> s) { return s.data()+s.len(); }
template <typename T>
T *begin(slice<T> s) { return s.data(); }
template <typename T>
T *end(slice<T> s) { return s.data()+s.len(); }

} // namespace zbs
