#pragma once

#include <cstring>
#include <algorithm>
#include <initializer_list>
#include "_utils.hh"

namespace zbs {

//============================================================================
// slice template helper macro
//============================================================================

// for const T
#define _common_slice_part_const(T)					\
private:								\
	const T *_data;							\
	int _len;							\
									\
public:									\
	slice(): _data(nullptr), _len(0) {}				\
	slice(std::initializer_list<T> r):				\
		_data(r.begin()), _len(r.size()) {}			\
	slice(const T *data, int len): _data(data), _len(len) {}	\
	slice(const slice<T> &r): _data(r.data()), _len(r.len()) {}	\
	slice(const slice &r) = default;				\
	slice &operator=(const slice &r) = default;			\
	explicit operator bool() const { return _data != nullptr; }     \
	int len() const { return _len; }				\
	const T *data() const { return _data; }				\
	slice<const T> sub() const {					\
		return {_data, _len};					\
	}								\
	slice<const T> sub(int begin) const {				\
		_ZBS_BOUNDS_CHECK(begin, _len);				\
		return {_data + begin, _len - begin};			\
	}								\
	slice<const T> sub(int begin, int end) const {			\
		_ZBS_ASSERT(begin < end);				\
		_ZBS_BOUNDS_CHECK(begin, _len);				\
		_ZBS_BOUNDS_CHECK(end, _len+1);				\
		return {_data + begin, end - begin};			\
	}								\
	const T &operator[](int idx) const {				\
		_ZBS_BOUNDS_CHECK(idx, _len);				\
		return _data[idx];					\
	}								\
private:

//============================================================================
// slice template
//============================================================================

template <typename T>
class slice {
	T *_data;
	int _len;

public:
	slice(): _data(nullptr), _len(0) {}
	slice(T *data, int len): _data(data), _len(len) {}
	slice(const slice &r) = default;
	slice &operator=(const slice &r) = default;
	explicit operator bool() const { return _data != nullptr; }

	T &operator[](int idx) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}

	const T &operator[](int idx) const {
		_ZBS_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}

	int len() const { return _len; }
	T *data() { return _data; }
	const T *data() const { return _data; }

	slice sub() {
		return {_data, _len};
	}
	slice sub(int begin) {
		_ZBS_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}
	slice sub(int begin, int end) {
		_ZBS_ASSERT(begin <= end);
		_ZBS_BOUNDS_CHECK(begin, _len);
		_ZBS_BOUNDS_CHECK(end, _len+1);
		return {_data + begin, end - begin};
	}
	slice<const T> sub() const {
		return {_data, _len};
	}
	slice<const T> sub(int begin) const {
		_ZBS_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}
	slice<const T> sub(int begin, int end) const {
		_ZBS_ASSERT(begin <= end);
		_ZBS_BOUNDS_CHECK(begin, _len);
		_ZBS_BOUNDS_CHECK(end, _len+1);
		return {_data + begin, end - begin};
	}
};

//============================================================================
// const slice template specialization
//
// it has an additional copy constructor from non-const slice<T>
//============================================================================
template <typename T>
class slice<const T> {
	_common_slice_part_const(T);
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

template <>
class slice<const char> {
	_common_slice_part_const(char)
public:
	slice(const char *str): _data(str), _len(::strlen(str)) {}
};

// For char16_t and char32_t there is no other choice than relying on good
// constexpr optimizations.
template <>
class slice<const char16_t> {
	_common_slice_part_const(char16_t)
	static constexpr int _strlen16(const char16_t *s) {
		return *s ? (1 + _strlen16(s+1)) : 0;
	}
public:
	constexpr slice(const char16_t *str): _data(str), _len(_strlen16(str)) {}
};

template <>
class slice<const char32_t> {
	_common_slice_part_const(char32_t)
	static constexpr int _strlen32(const char32_t *s) {
		return *s ? (1 + _strlen32(s+1)) : 0;
	}
public:
	constexpr slice(const char32_t *str): _data(str), _len(_strlen32(str)) {}
};

#undef _common_slice_part_const

//============================================================================
// slice comparison operators
//============================================================================

template <typename T>
bool operator==(slice<T> lhs, slice<T> rhs) {
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

template <typename T>
bool operator<(slice<T> lhs, slice<T> rhs) {
	const int len = std::min(lhs.len(), rhs.len());
	for (int i = 0; i < len; i++) {
		if (lhs.data()[i] < rhs.data()[i]) {
			return true;
		}
	}
	return lhs.len() < rhs.len();
}

template <typename T>
bool operator!=(slice<T> lhs, slice<T> rhs) { return !operator==(lhs, rhs); }
template <typename T>
bool operator>=(slice<T> lhs, slice<T> rhs) { return !operator<(lhs, rhs); }
template <typename T>
bool operator<=(slice<T> lhs, slice<T> rhs) { return !operator<(rhs, lhs); }
template <typename T>
bool operator>(slice<T> lhs, slice<T> rhs) { return operator<(rhs, lhs); }

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

} // namespace zbs
