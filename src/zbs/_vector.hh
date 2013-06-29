#pragma once

#include <initializer_list>
#include <limits>
#include <new>
#include <algorithm>

#include "_utils.hh"
#include "_slice.hh"

namespace zbs {

/// Container template, provides support for dynamically growing arrays.
///
/// It's safe to assume that elements are stored sequentially in memory, thus
/// it is possible to interact with pointer-based array APIs.
///
/// The amount of memory vector allocates is usually more than it is required
/// to hold all the elements. It's done that way for a purpose of handling
/// future element growth. The total amount of elements the vector may hold
/// without performing reallocations can be queried using the cap() method. If
/// you need to return all extra memory to the system, just call the shrink()
/// method.
///
/// @headerfile zbs/vector.hh
///
/// TODO(nsf): better description here
template <typename T>
class vector {
	T *_data;
	int _len;
	int _cap;

	inline int _new_size(int requested) const {
		const int max = std::numeric_limits<int>::max();
		if (_cap > max) {
			return max;
		}
		return std::max(_cap * 2, requested);
	}

	// expects: idx < _len, idx >= 0, offset > 0
	inline void _move_forward(int idx, int offset) {
		const int last = _len-1;
		int src = last;
		int dst = last+offset;
		while (src >= idx) {
			new (&_data[dst]) T(std::move(_data[src]));
			_data[src].~T();
			src--;
			dst--;
		}
	}

	// expects: idx < _len, idx >= 0, offset < 0
	inline void _move_backward(int idx, int offset) {
		int src = idx;
		int dst = idx+offset;
		while (src < _len) {
			new (&_data[dst]) T(std::move(_data[src]));
			_data[src].~T();
			src++;
			dst++;
		}
	}

	// ensure that there is enough capacity to hold _len + n items
	inline void _ensure_capacity(int n) {
		if (_len + n > _cap) {
			reserve(_new_size(_len + n));
		}
	}

	// this one is called when `s` points to the vector itself
	inline void _self_insert(int idx, slice<const T> s) {
		// slice data index, we need it because slice will be
		// invalidated (possibly) after _ensure_capacity call
		int sidx = s.data() - _data;
		_ensure_capacity(s.len());
		s = slice<const T>(_data + sidx, s.len()); // restore slice
		if (idx == _len) {
			// shortcut for append
			for (int i = 0; i < s.len(); i++) {
				new (&_data[idx+i]) T(s.data()[i]);
			}
			_len += s.len();
			return;
		}

		_move_forward(idx, s.len());
		_len += s.len();
		if (idx <= sidx) {
			// slice points somewhere after insertion place, adjust
			// the slice according to _move_forward result
			s = slice<const T>(s.data() + s.len(), s.len());
		} else if (sidx < idx && idx < sidx + s.len()) {
			// the most complicated case, idx is somewhere on the
			// slice (not at the beginning of the slice), means we
			// need to split the slice into two and insert them
			// separately
			const int lhslen = idx - sidx;
			for (int i = 0; i < lhslen; i++) {
				new (&_data[idx+i]) T(_data[sidx+i]);
			}
			idx += lhslen;
			s = slice<const T>(s.data() + s.len() + lhslen, s.len() - lhslen);
		}
		for (int i = 0; i < s.len(); i++) {
			new (&_data[idx+i]) T(s.data()[i]);
		}
	}

public:
	/// Default constructor. Constructs an empty vector.
	vector(): _data(nullptr), _len(0), _cap(0) {}

	/// Uses a copy of the contents of the initializer list to construct a
	/// vector.
	vector(std::initializer_list<T> r): vector(slice<const T>(r)) {}

	/// Copy constructor. Constructs a vector with a copy of the contents
	/// of `r`.
	vector(const vector &r): vector(slice<const T>(r)) {}

	/// Move constructor. Uses the contents of `r` to construct a vector.
	vector(vector &&r): _data(r._data), _len(r._len), _cap(r._cap) {
		r._data = nullptr;
		r._len = 0;
		r._cap = 0;
	}

	/// Uses a copy of the contents of the slice `r` to construct a vector.
	vector(slice<const T> r): _data(nullptr), _len(r.len()), _cap(r.len()) {
		if (_len == 0) {
			return;
		}
		_data = detail::malloc<T>(_cap);
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(r.data()[i]);
		}
	}

	/// Uses a copy of the contents of the slice `r` to construct a vector.
	vector(slice<T> r): vector(slice<const T>(r)) {}

	/// Constructs a vector with `n` default-constructed values.
	explicit vector(int n): _data(nullptr), _len(n), _cap(n) {
		_ZBS_ASSERT(n >= 0);
		if (_len == 0) {
			return;
		}
		_data = detail::malloc<T>(_cap);
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T;
		}
	}

	/// Constructs a vector with `n` copy-constructed values using `elem`
	/// as a prototype.
	vector(int n, const T &elem): _data(nullptr), _len(n), _cap(n) {
		_ZBS_ASSERT(n >= 0);
		if (_len == 0) {
			return;
		}
		_data = detail::malloc<T>(_cap);
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(elem);
		}
	}

	/// Destruct all the elements of the vector and deallocate used storage.
	~vector() {
		for (int i = 0; i < _len; i++) {
			_data[i].~T();
		}
		detail::free(_data);
	}

	/// Replaces the contents of the vector with a copy of the contents of
	/// the slice `r`.
	vector &operator=(slice<const T> r) {
		if (_data == r.data() && _len == r.len()) {
			// self copy shortcut (a = a)
			return *this;
		}
		if (_cap < r.len()) {
			// slice is bigger than we are, realloc needed, also it
			// means slice cannot point to ourselves and it is save
			// to destroy ourselves
			for (int i = 0; i < _len; i++) {
				_data[i].~T();
			}
			detail::free(_data);
			_cap = _len = r.len();
			_data = detail::malloc<T>(_cap);
			for (int i = 0; i < _len; i++) {
				new (&_data[i]) T(r.data()[i]);
			}
		} else {
			// slice can be a subset of ourselves
			int i = copy(sub(), r);
			for (; i < _len; i++) {
				// destroy the rest if any
				_data[i].~T();
			}
			for (; i < r.len(); i++) {
				// construct the new if any
				new (&_data[i]) T(r.data()[i]);
			}
			_len = r.len();
		}
		return *this;
	}

	/// Replaces the contents of the vector with a copy of the contents of
	/// the slice `r`.
	vector &operator=(slice<T> r) {
		return operator=(slice<const T>(r));
	}

	/// Replaces the contents of the vector with a copy of the contents of
	/// the initializer list `r`.
	vector &operator=(std::initializer_list<T> r) {
		return operator=(slice<const T>(r));
	}

	/// Replaces the contents of the vector with a copy of the contents of
	/// the vector `r`.
	vector &operator=(const vector &r) {
		return operator=(slice<const T>(r));
	}

	/// Replaces the contents of the vector with the contents of the vector
	/// `r`.
	vector &operator=(vector &&r) {
		if (&r == this) {
			return *this;
		}
		for (int i = 0; i < _len; i++) {
			_data[i].~T();
		}
		detail::free(_data);
		_data = r._data;
		_len = r._len;
		_cap = r._cap;
		r._data = nullptr;
		r._len = 0;
		r._cap = 0;
		return *this;
	}

	/// Detaches the data (if any) from the vector. The caller is
	/// responsible for freeing it with detail::free() afterwards. It is
	/// useful in certain cases, like for example moving memory from vector
	/// to string. Sure it's possible to add move constructors and move
	/// assignment operators for these cases, but it introduces
	/// dependencies between otherwise independent types.
	/// @unsafe
	inline T *detach_unsafe() {
		T *data = _data;
		_data = nullptr;
		_len = 0;
		_cap = 0;
		return data;
	}

	/// Uses provided arguments as internal data, releasing all the
	/// previous elements of the vector. The data should be previously
	/// allocated with detail::malloc().
	/// @unsafe
	inline void attach_unsafe(T *data, int len, int cap) {
		clear();
		shrink();
		_data = data;
		_len = len;
		_cap = cap;
	}

	/// Returns an amount of active elements the vector holds at a given
	/// moment.
	inline int len() const { return _len; }

	/// Returns an amount of elements the vector may hold without
	/// performing reallocations.
	inline int cap() const { return _cap; }

	/// Returns a pointer to the first element in the vector.
	inline T *data() { return _data; }

	/// Returns a pointer to the first element in the vector.
	inline const T *data() const { return _data; }

	/// Clears the vector by deconstructing all of its elements.
	///
	/// Doesn't reallocate or release memory, but the len() is set to zero.
	void clear() {
		for (int i = 0; i < _len; i++) {
			_data[i].~T();
		}
		_len = 0;
	}

	/// Makes sure there is enough place to hold `n` elements.
	///
	/// The function will reallocate the vector if the current cap() is
	/// less that what was requested. Negative requests are ignored.
	///
	/// Reallocation is not a cheap operation and if you want to fill a
	/// vector with a known (even if approximately) amount of values, use
	/// this method for that purpose.
	void reserve(int n) {
		if (_cap >= n) {
			return;
		}

		T *old_data = _data;
		_cap = n;
		_data = detail::malloc<T>(_cap);
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(std::move(old_data[i]));
			old_data[i].~T();
		}
		detail::free(old_data);
	}

	/// Releases unused memory to the system.
	///
	/// If the cap() != len(), it will reallocate the vector so that cap() == len().
	void shrink() {
		if (_cap == _len) {
			return;
		}

		T *old_data = _data;
		_cap = _len;
		if (_len > 0) {
			_data = detail::malloc<T>(_cap);
			for (int i = 0; i < _len; i++) {
				new (&_data[i]) T(std::move(old_data[i]));
				old_data[i].~T();
			}
		} else {
			_data = nullptr;
		}
		detail::free(old_data);
	}

	/// Resizes the vector to contain `n` elements.
	///
	/// If `n` is greater than len(), additional elements are appended and
	/// default-constructed.
	///
	/// If `n` is less than len(), the vector is reduced to first `n`
	/// elements.
	void resize(int n) {
		_ZBS_ASSERT(n >= 0);
		if (_len == n) {
			return;
		}

		if (_len > n) {
			for (int i = n; i < _len; i++) {
				_data[i].~T();
			}
			_len = n;
			return;
		}

		reserve(n);
		for (int i = _len; i < n; i++) {
			new (&_data[i]) T;
		}
		_len = n;
	}

	/// Resizes the vector to contain `n` elements.
	///
	/// If `n` is greater than len(), additional elements are appended and
	/// copy-constructed using `elem` as a prototype.
	///
	/// If `n` is less than len(), the vector is reduced to first `n`
	/// elements.
	void resize(int n, const T &elem) {
		// copy & paste from resize(int)
		_ZBS_ASSERT(n >= 0);
		if (_len == n) {
			return;
		}

		if (_len > n) {
			for (int i = n; i < _len; i++) {
				_data[i].~T();
			}
			_len = n;
			return;
		}

		reserve(n);
		for (int i = _len; i < n; i++) {
			new (&_data[i]) T(elem);
		}
		_len = n;
	}

	/// Constructs a new element in the specified position.
	///
	/// The name is an abbreviation for placement insert. It uses C++11
	/// variadic templates and perfect forwarding features to construct a
	/// new element in a given place `idx`. If `idx` == len(), then the
	/// element is simply appended.
	template <typename ...Args>
	inline void pinsert(int idx, Args &&...args) {
		_ZBS_SLICE_BOUNDS_CHECK(idx, _len);
		_ensure_capacity(1);
		if (idx < _len) {
			_move_forward(idx, 1);
		}
		new (&_data[idx]) T(std::forward<Args>(args)...);
		_len++;
	}

	/// Constructs a new element in-place at the end of the vector.
	///
	/// The name is an abbreviation for placement append. It uses C++11
	/// variadic templates and perfect forwarding features to construct a
	/// new element at the end of the vector.
	template <typename ...Args>
	inline void pappend(Args &&...args) {
		_ensure_capacity(1);
		new (&_data[_len]) T(std::forward<Args>(args)...);
		_len++;
	}

	/// Removes an element at the specified position `idx`.
	inline void remove(int idx) {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		if (idx == _len - 1) {
			_data[_len-1].~T();
			_len--;
			return;
		}

		_data[idx].~T();
		_move_backward(idx+1, -1);
		_len--;
	}

	/// Inserts a copy of the elements from the slice `s` at the specified
	/// position `idx`.
	///
	/// It is safe to use a slice of the vector itself to perform the
	/// insertion.
	inline void insert(int idx, slice<const T> s) {
		_ZBS_SLICE_BOUNDS_CHECK(idx, _len);
		if (s.len() == 0) {
			return;
		}
		if (s.data() >= _data && s.data() < _data + _len) {
			// if slice points to ourselves, use special magic
			_self_insert(idx, s);
			return;
		}
		_ensure_capacity(s.len());
		if (idx < _len) {
			_move_forward(idx, s.len());
		}
		for (int i = 0; i < s.len(); i++) {
			new (&_data[idx+i]) T(s.data()[i]);
		}
		_len += s.len();
	}

	/// Inserts a copy of the element `elem` at the specified position
	/// `idx`.
	inline void insert(int idx, const T &elem) {
		pinsert(idx, elem);
	}

	/// Inserts the element `elem` at the specified position `idx`.
	inline void insert(int idx, T &&elem) {
		pinsert(idx, elem);
	}

	/// Appends a copy of the elements from the slice `s` to the end of the
	/// vector.
	inline void append(slice<const T> s) {
		insert(_len, s);
	}

	/// Appends a copy of the element `elem` to the end of the vector.
	inline void append(const T &elem) {
		pappend(elem);
	}

	/// Appends the element `elem` to the end of the vector.
	inline void append(T &&elem) {
		pappend(elem);
	}

	/// Removes the slice [`begin`, `end`) of elements from the vector.
	///
	/// All elements between `begin` (inclusive) and `end` (exclusive) are
	/// deconstructed. It's easier to remember the meaning of parameters if
	/// you realize that the amount of removed elements equals to
	/// (`end` - `begin`).
	inline void remove(int begin, int end) {
		_ZBS_ASSERT(begin <= end);
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);
		const int len = end - begin;
		if (len == 0) {
			return;
		}
		for (int i = begin; i < end; i++) {
			_data[i].~T();
		}
		if (end < _len) {
			_move_backward(begin+len, -len);
		}
		_len -= len;
	}

	/// Returns the slice [0, len()) of the vector.
	inline slice<T> sub() {
		return {_data, _len};
	}

	/// Returns the slice [`begin`, len()) of the vector.
	inline slice<T> sub(int begin) {
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}

	/// Returns the slice [`begin`, `end`) of the vector.
	inline slice<T> sub(int begin, int end) {
		_ZBS_ASSERT(begin <= end);
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);
		return {_data + begin, end - begin};
	}

	/// Returns the slice [0, len()) of the vector.
	inline slice<const T> sub() const {
		return {_data, _len};
	}

	/// Returns the slice [`begin`, len()) of the vector.
	inline slice<const T> sub(int begin) const {
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}

	/// Returns the slice [`begin`, `end`) of the vector.
	inline slice<const T> sub(int begin, int end) const {
		_ZBS_ASSERT(begin <= end);
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);
		return {_data + begin, end - begin};
	}

	/// Typical element access operator.
	inline T &operator[](int idx) {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}

	/// Typical element access operator.
	inline const T &operator[](int idx) const {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}

	/// Vector to slice implicit conversion operator.
	inline operator slice<T>() { return {_data, _len}; }

	/// Vector to slice implicit conversion operator.
	inline operator slice<const T>() const { return {_data, _len}; }
};

/// @cppforeach @relates zbs::vector
template <typename T>
const T *begin(const vector<T> &v) { return v.data(); }
/// @cppforeach @relates zbs::vector
template <typename T>
const T *end(const vector<T> &v) { return v.data()+v.len(); }
/// @cppforeach @relates zbs::vector
template <typename T>
T *begin(vector<T> &v) { return v.data(); }
/// @cppforeach @relates zbs::vector
template <typename T>
T *end(vector<T> &v) { return v.data()+v.len(); }

} // namespace zbs
