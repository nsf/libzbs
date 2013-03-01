#pragma once

#include <cstring>
#include <algorithm>
#include <initializer_list>
#include "_utils.hh"

namespace zbs {


// slice template helper macro, however we unwrap it manually in the `const T`
// slice specialization for documentation purposes, keep that in mind if you're
// editing it
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
	explicit operator bool() const { return _len != 0; }            \
	int len() const { return _len; }				\
	const T *data() const { return _data; }				\
	slice<const T> sub() const {					\
		return {_data, _len};					\
	}								\
	slice<const T> sub(int begin) const {				\
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);			\
		return {_data + begin, _len - begin};			\
	}								\
	slice<const T> sub(int begin, int end) const {			\
		_ZBS_ASSERT(begin <= end);				\
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);			\
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);			\
		return {_data + begin, end - begin};			\
	}								\
	const T &operator[](int idx) const {				\
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);			\
		return _data[idx];					\
	}								\
private:

/// Reference to a contiguous segment of an array.
///
/// Slice is one of the fundamental concepts in the libzbs. You can think of it
/// as a safe pointer to a segment of an array. Not only it contains a pointer
/// to the first element, it also keeps the length of that segment. Using
/// slices everywhere is a very convenient way to avoid various kinds of buffer
/// overflow bugs.
///
/// Slice explicitly doesn't have any kind of memory ownership semantics, it's
/// up to you to keep an eye on who's the owner of the memory and when it goes
/// away. In that regard it's as safe as an ordinary pointer. On the other hand
/// using slice we can unify access to all sorts of seqential structures, be it
/// a temporary C array on the stack, or a zbs::vector, or a zbs::string, or
/// even an initalizer list.
template <typename T>
class slice {
	T *_data;
	int _len;

public:

	/// Default constructor. Constructs an empty slice.
	///
	/// Technically a null slice and an empty slice are semantically
	/// equivalent, because you can't do anything useful with slice's data
	/// if its length is zero.
	slice(): _data(nullptr), _len(0) {}

	/// Constructs a slice using raw data pointer and a length.
	/// @unsafe
	slice(T *data, int len): _data(data), _len(len) {}

	/// Copy constructor. Constructs a slice using the data pointer and the
	/// length from the slice `r`.
	slice(const slice &r) = default;

	/// Replaces the contents of the slice with the data pointer and the
	/// length from the slice `r`.
	slice &operator=(const slice &r) = default;

	/// Checks if the slice is empty or not.
	explicit operator bool() const { return _len != 0; }

	/// Typical element access operator.
	T &operator[](int idx) {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}

	/// Typical element access operator.
	const T &operator[](int idx) const {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}

	/// Returns the length of the slice.
	int len() const { return _len; }

	/// Returns a pointer to the first element of the slice.
	T *data() { return _data; }

	/// Returns a pointer to the first element of the slice.
	const T *data() const { return _data; }

	/// Returns the subslice [0, len()) of the slice.
	slice sub() {
		return {_data, _len};
	}

	/// Returns the subslice [`begin`, len()) of the slice.
	slice sub(int begin) {
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}

	/// Returns the subslice [`begin`, `end`) of the slice.
	slice sub(int begin, int end) {
		_ZBS_ASSERT(begin <= end);
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);
		return {_data + begin, end - begin};
	}

	/// Returns the subslice [0, len()) of the slice.
	slice<const T> sub() const {
		return {_data, _len};
	}

	/// Returns the subslice [`begin`, len()) of the slice.
	slice<const T> sub(int begin) const {
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}

	/// Returns the subslice [`begin`, `end`) of the slice.
	slice<const T> sub(int begin, int end) const {
		_ZBS_ASSERT(begin <= end);
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);
		return {_data + begin, end - begin};
	}
};

/// Slice template specialization for `const T`.
/// @sa slice
template <typename T>
class slice<const T> {
private:
	const T *_data;
	int _len;

public:
	/// @copydoc slice::slice()
	slice(): _data(nullptr), _len(0) {}

	/// Constructs a slice using std::initializer_list.
	///
	/// This function is only available for `const T` specialization of the
	/// slice for obvious reasons: std::initializer_list represents a const
	/// C array.
	///
	/// @warning However, never use this constructor to create a slice
	/// object on the stack like that:
	/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
	/// zbs::slice<const int> = { 1, 2, 3, 4, 5 };
	/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/// It's supposed to be used only as a syntax sugar for creating
	/// temporary slices, e.g.:
	/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
	/// if (zbs::bytes::starts_with(slice, {'\xFF', '\xFE'})) {}
	/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/// Perhaps it's not dangerious to do so with POD types, but when it
	/// comes to non-POD types, the std::initializer_list will be destroyed
	/// (destroying its elements as well) immediately after construction of
	/// a slice object is finished. And as a result the newly created slice
	/// contains a pointer to destroyed invalid data.
	///
	/// @warning We have this constructor solely for the purpose of
	/// allowing the usage of std::initializer_list in all the functions
	/// which take slice<const T> as an argument.
	slice(std::initializer_list<T> r):
		_data(r.begin()), _len(r.size()) {}

	/// @copydoc slice::slice(T*, int)
	slice(const T *data, int len): _data(data), _len(len) {}

	/// Constructs a slice using a copy of the non-const slice `r` of the
	/// same type.
	slice(const slice<T> &r): _data(r.data()), _len(r.len()) {}

	/// @copydoc slice::slice(const slice&)
	slice(const slice &r) = default;

	/// @copydoc slice::operator=(const slice&)
	slice &operator=(const slice &r) = default;

	/// @copydoc slice::operator bool() const
	explicit operator bool() const { return _len != 0; }

	/// @copydoc slice::len()
	int len() const { return _len; }

	/// @copydoc slice::data() const
	const T *data() const { return _data; }

	/// @copydoc slice::sub() const
	slice<const T> sub() const {
		return {_data, _len};
	}

	/// @copydoc slice::sub(int) const
	slice<const T> sub(int begin) const {
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}

	/// @copydoc slice::sub(int, int) const
	slice<const T> sub(int begin, int end) const {
		_ZBS_ASSERT(begin <= end);
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);
		return {_data + begin, end - begin};
	}

	/// @copydoc slice::operator[](int) const
	const T &operator[](int idx) const {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}
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
public:
	/// Constructs a slice using the null-terminated C string `str`.
	slice(const char *str): _data(str), _len(::strlen(str)) {}
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

/// Copy the slice `src` to the slice `dst`. Function copies the
/// `min(dst.len(), src.len())` elements and returns the number of elements
/// copied. It's is safe to use this function for overlapping slices.
/// @relates zbs::slice
template <typename T, typename U, bool is_pod = std::is_pod<T>::value>
int copy(slice<T> dst, slice<U> src) {
	_ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(T, U);
	return detail::copier<T, U, is_pod>::copy(dst, src);
}

/// @cppforeach @relates zbs::slice
template <typename T>
const T *begin(slice<const T> s) { return s.data(); }
/// @cppforeach @relates zbs::slice
template <typename T>
const T *end(slice<const T> s) { return s.data()+s.len(); }
/// @cppforeach @relates zbs::slice
template <typename T>
T *begin(slice<T> s) { return s.data(); }
/// @cppforeach @relates zbs::slice
template <typename T>
T *end(slice<T> s) { return s.data()+s.len(); }

} // namespace zbs
