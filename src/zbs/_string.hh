#pragma once

#include <cstring>
#include <limits>
#include <algorithm>

#include "_utils.hh"
#include "_slice.hh"

namespace zbs {

namespace detail {

template <typename T>
struct char_traits {
	static T *empty_string() {
		static T null = 0;
		return &null;
	}
};

} // namespace detail

//============================================================================
// basic string template
//============================================================================

template <typename T>
class basic_string {
protected:
	T *_data = detail::char_traits<T>::empty_string();
	int _len = 0;
	int _cap = 0;

	inline int _new_size(int requested) const {
		const int max = std::numeric_limits<int>::max();
		if (_cap > max) {
			return max;
		}
		return std::max(_cap * 2, requested);
	}

	// ensure that there is enough capacity to hold _len + n items
	inline void _ensure_capacity(int n) {
		if (_len + n > _cap) {
			reserve(_new_size(_len + n));
		}
	}

	inline void _self_insert(int idx, slice<const T> s) {
		int sidx = s.data() - _data;
		_ensure_capacity(s.len());
		s = slice<const T>(_data + sidx, s.len()); // restore slice
		if (idx == _len) {
			::memcpy(_data + idx, s.data(), s.len() * sizeof(T));
			_len += s.len();
			_data[_len] = 0;
			return;
		}

		::memmove(_data + idx + s.len(), _data + idx, (_len - idx) * sizeof(T));
		_len += s.len();
		if (idx <= sidx) {
			s = slice<const T>(s.data() + s.len(), s.len());
		} else {
			const int lhslen = idx - sidx;
			::memmove(_data + idx, _data + sidx, lhslen * sizeof(T));
			idx += lhslen;
			s = slice<const T>(s.data() + s.len() + lhslen, s.len() - lhslen);
		}
		::memmove(_data + idx, s.data(), s.len() * sizeof(T));
		_data[_len] = 0;
	}

public:
	basic_string() {}

	basic_string(slice<const T> r): _len(r.len()), _cap(r.len()) {
		if (_len == 0) {
			return;
		}
		_data = detail::malloc<T>(_cap + 1);
		::memcpy(_data, r.data(), r.len() * sizeof(T));
		_data[_len] = 0;
	}

	basic_string(const basic_string &r): basic_string(slice<const T>(r.sub())) {}

	basic_string(basic_string &&r): _data(r._data), _len(r._len), _cap(r._cap) {
		r._data = detail::char_traits<T>::empty_string();
		r._len = 0;
		r._cap = 0;
	}

	~basic_string() {
		// _data is always != nullptr, we can't use it to see it the
		// memory was actually allocated
		if (_data != detail::char_traits<T>::empty_string()) {
			detail::free(_data);
		}
	}

	basic_string &operator=(slice<const T> r) {
		if (_data == r.data() && _len == r.len()) {
			// self copy shortcut (a = a)
			return *this;
		}
		if (_cap < r.len()) {
			if (_data != detail::char_traits<T>::empty_string()) {
				detail::free(_data);
			}
			_cap = _len = r.len();
			_data = detail::malloc<T>(_cap + 1);
			::memcpy(_data, r.data(), _len * sizeof(T));
			_data[_len] = 0;
		} else {
			_len = r.len();
			::memmove(_data, r.data(), _len * sizeof(T));
			if (_data != detail::char_traits<T>::empty_string()) {
				_data[_len] = 0;
			}
		}
		return *this;
	}

	basic_string &operator=(const basic_string &r) {
		return operator=(slice<const T>(r.sub()));
	}

	basic_string &operator=(basic_string &&r) {
		if (_data != detail::char_traits<T>::empty_string()) {
			detail::free(_data);
		}
		_data = r._data;
		_len = r._len;
		_cap = r._cap;
		r._data = detail::char_traits<T>::empty_string();
		r._len = 0;
		r._cap = 0;
		return *this;
	}

	inline T *detach_unsafe() {
		T *data = _data;
		_data = detail::char_traits<T>::empty_string();
		_len = 0;
		_cap = 0;
		return data;
	}

	inline void attach_unsafe(T *data, int len, int cap) {
		clear();
		shrink();
		_data = data;
		_len = len;
		_cap = cap;
	}

	int len() const { return _len; }
	int byte_len() const { return _len * sizeof(T); }
	int cap() const { return _cap; }
	T *data() { return _data; }
	const T *data() const { return _data; }
	const T *c_str() const { return _data; }

	void clear() {
		_len = 0;
		if (_data != detail::char_traits<T>::empty_string()) {
			_data[0] = 0;
		}
	}

	void reserve(int n) {
		if (_cap >= n) {
			return;
		}

		T *old_data = _data;
		_cap = n;
		_data = detail::malloc<T>(_cap + 1);
		if (_len > 0) {
			// copy terminating zero as well
			::memcpy(_data, old_data, (_len + 1) * sizeof(T));
		} else {
			_data[0] = 0;
		}
		if (old_data != detail::char_traits<T>::empty_string()) {
			detail::free(old_data);
		}
	}

	void shrink() {
		if (_cap == _len) {
			return;
		}

		T *old_data = _data;
		if (_len > 0) {
			_data = detail::malloc<T>(_len + 1);
			::memcpy(_data, old_data, (_len + 1) * sizeof(T));
		} else {
			_data = detail::char_traits<T>::empty_string();
		}
		_cap = _len;
		if (old_data != detail::char_traits<T>::empty_string()) {
			detail::free(old_data);
		}
	}

	void resize(int n) {
		_ZBS_ASSERT(n >= 0);
		if (_len == n) {
			return;
		}

		if (_len > n) {
			_len = n;
			_data[_len] = 0;
			return;
		}

		reserve(n);
		::memset(_data + _len, 0, (n - _len + 1) * sizeof(T));
		_len = n;
	}

	void resize(int n, T elem) {
		_ZBS_ASSERT(n >= 0);
		if (_len == n) {
			return;
		}

		if (_len > n) {
			_len = n;
			_data[_len] = 0;
			return;
		}

		reserve(n);
		for (int i = _len; i < n; i++) {
			_data[i] = elem;
		}
		_len = n;
		_data[_len] = 0;
	}

	void insert(int idx, T elem) {
		_ZBS_SLICE_BOUNDS_CHECK(idx, _len);
		_ensure_capacity(1);
		if (idx < _len) {
			T *dst = _data + idx;
			::memmove(dst + 1, dst, (_len - idx) * sizeof(T));
		}
		_data[idx] = elem;
		_data[++_len] = 0;
	}

	void append(T elem) {
		_ensure_capacity(1);
		_data[_len++] = elem;
		_data[_len] = 0;
	}

	void remove(int idx) {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		if (idx == _len - 1) {
			_data[--_len] = 0;
			return;
		}
		T *dst = _data + idx;
		::memmove(dst, dst+1, (_len - (idx+1)) * sizeof(T));
		_data[--_len] = 0;
	}

	void insert(int idx, slice<const T> s) {
		_ZBS_SLICE_BOUNDS_CHECK(idx, _len);
		if (s.len() == 0) {
			return;
		}
		if (s.data() >= _data && s.data() < _data + _len) {
			_self_insert(idx, s);
			return;
		}
		_ensure_capacity(s.len());
		T *dst = _data + idx;
		::memmove(dst + s.len(), dst, (_len - idx) * sizeof(T));
		::memcpy(dst, s.data(), s.len() * sizeof(T));
		_len += s.len();
		_data[_len] = 0;

	}

	void append(slice<const T> s) {
		insert(_len, s);
	}

	void remove(int begin, int end) {
		_ZBS_ASSERT(begin <= end);
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		_ZBS_SLICE_BOUNDS_CHECK(end, _len);
		const int len = end - begin;
		if (end < _len) {
			::memmove(_data + begin, _data + end,
				(_len - end) * sizeof(T));
		}
		_len -= len;
		_data[_len] = 0;
	}

	slice<T> sub() {
		return {_data, _len};
	}

	slice<T> sub(int begin) {
		_ZBS_SLICE_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}

	slice<T> sub(int begin, int end) {
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

extern template class basic_string<char>;
extern template class basic_string<char16_t>;
extern template class basic_string<char32_t>;

//============================================================================
// utf-8 string
//============================================================================

/// Container for a null-terminated sequence of bytes. Many methods and
/// functions assume its contents are UTF-8 encoded. There are no invalid
/// strings, all strings are valid in a sense that they are a sequence of zero
/// or more bytes ending with a null byte.
class string : public basic_string<char> {
public:
	/// Default constructor. Constructs an empty string.
	string() = default;

	/// Copy constructor. Constructs a copy of another string.
	string(const string&) = default;

	/// Move constructor. Uses the contents of another string to construct
	/// a string.
	string(string&&) = default;
	~string() = default;
	string &operator=(const string&) = default;
	string &operator=(string&&) = default;

public:
	string(slice<const char> r);
	string(const char *cstr);
	string &operator=(slice<const char> r);
	string &operator=(const char *cstr);
	char &operator[](int idx) {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}
	const char &operator[](int idx) const {
		_ZBS_IDX_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}
	operator slice<char>() { return sub(); }
	operator slice<const char>() const { return sub(); }
};

// string vs. const char*
string operator+(const string &lhs, const char *rhs);
string operator+(const char *lhs, const string &rhs);
string operator+(string &&lhs, const char *rhs);
string operator+(const char *lhs, string &&rhs);

// string vs. string
string operator+(const string &lhs, const string &rhs);
string operator+(string &&lhs, string &&rhs);
string operator+(string &&lhs, const string &rhs);
string operator+(const string &lhs, string &&rhs);

// string vs. slice<const char>
string operator+(const string &lhs, slice<const char> rhs);
string operator+(slice<const char> lhs, const string &rhs);
string operator+(string &&lhs, slice<const char> rhs);
string operator+(slice<const char> lhs, string &&rhs);

// hash specialization for string
template <typename T>
struct hash;

template <>
struct hash<string> {
	int operator()(const string &s, int seed);
};

// for each loop support
struct rune_and_offset {
	zbs::rune rune;
	int offset;
};

class string_iter {
	slice<const char> _s;
	rune _r;
	int _offset;

public:
	string_iter() = default;
	explicit string_iter(slice<const char> s);
	string_iter &operator++();
	bool operator==(const string_iter &r) const;
	bool operator!=(const string_iter &r) const;
	rune_and_offset operator*() const;
};

// these are useful for manual init, like:
// for (const auto &iter : string_iter(s.sub(x))) {}
static inline string_iter begin(string_iter si) { return si; }
static inline string_iter end(string_iter si) { return si; }

string_iter begin(const string &s);
string_iter end(const string &s);


} // namespace zbs
