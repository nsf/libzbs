#pragma once

#include <cstdlib>
#include <cstring>
#include <limits>
#include <algorithm>
#include "slice.hh"

namespace zbs {

namespace /* zbs:: */ detail {

template <typename T>
struct char_traits {
	static T *empty_string() {
		static T null = 0;
		return &null;
	}
};

} // namespace zbs::detail

//============================================================================
// basic string template
//============================================================================

template <typename T>
class basic_string {
protected:
	T *_data;
	int _len;
	int _cap; // the actual capacity is (_cap + 1) for terminating null

	int _new_size(int requested) const {
		const int max = std::numeric_limits<int>::max();
		if (_cap > max) {
			return max;
		}
		return std::max(_cap * 2, requested);
	}

public:
	basic_string(): _data(detail::char_traits<T>::empty_string()), _len(0), _cap(0) {}

	basic_string(const basic_string &r): _len(r._len), _cap(r._len) {
		_data = static_cast<T*>(malloc((_len + 1) * sizeof(T)));
		::memcpy(_data, r._data, (_len + 1) * sizeof(T));
	}

	basic_string(basic_string &&r): _data(r._data), _len(r._len), _cap(r._cap) {
		r._data = detail::char_traits<T>::empty_string();
		r._len = 0;
		r._cap = 0;
	}

	~basic_string() {
		// _data is always != nullptr, we can't use it to see it the
		// memory was actually allocated
		if (_cap > 0) {
			free(static_cast<void*>(_data));
		}
	}

	basic_string &operator=(const basic_string &r) {
		_len = 0;
		reserve(r._len);
		_len = r._len;
		::memcpy(_data, r._data, (_len + 1) * sizeof(T));
		return *this;
	}

	basic_string &operator=(basic_string &&r) {
		if (_cap > 0) {
			free(static_cast<void*>(_data));
		}
		_data = r._data;
		_len = r._len;
		_cap = r._cap;
		r._data = detail::char_traits<T>::empty_string();
		r._len = 0;
		r._cap = 0;
		return *this;
	}

	int len() const { return _len; }
	int cap() const { return _cap; }
	T *data() { return _data; }
	const T *data() const { return _data; }
	const T *c_str() const { return _data; }

	void clear() {
		_len = 0;
		if (_cap > 0) {
			_data[0] = 0;
		}
	}

	void reserve(int n) {
		if (_cap >= n) {
			return;
		}
		T *old_data = _data;
		_data = static_cast<T*>(malloc((n + 1) * sizeof(T)));
		if (_len > 0) {
			// copy terminating zero as well
			::memcpy(_data, old_data, (_len + 1) * sizeof(T));
		}
		if (_cap > 0) {
			free(static_cast<void*>(old_data));
		}
		_cap = n;
	}

	void shrink() {
		if (_cap == _len) {
			return;
		}

		T *old_data = _data;
		if (_len > 0) {
			_data = static_cast<T*>(::malloc((_len + 1) * sizeof(T)));
			::memcpy(_data, old_data, (_len + 1) * sizeof(T));
		} else {
			_data = detail::char_traits<T>::empty_string();
		}
		if (_cap > 0) {
			free(static_cast<void*>(old_data));
		}
		_cap = _len;
	}

	void resize(int n) {
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

	void insert_after(int idx, T elem) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (idx == _len - 1) {
			append(elem);
		} else {
			insert_before(idx+1, elem);
		}
	}

	void insert_before(int idx, T elem) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (_len + 1 > _cap) {
			reserve(_new_size(_len + 1));
		}
		T *src = _data + idx;
		T *dst = src + 1;
		::memmove(dst, src, (_len - idx) * sizeof(T));
		_data[idx] = elem;
		_data[++_len] = 0;
	}

	void append(T elem) {
		if (_len + 1 > _cap) {
			reserve(_new_size(_len + 1));
		}
		_data[_len++] = elem;
		_data[_len] = 0;
	}

	void remove(int idx) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (idx == _len - 1) {
			_data[--_len] = 0;
			return;
		}
		T *dst = _data + idx;
		T *src = dst + 1;
		::memmove(dst, src, (_len - (idx+1)) * sizeof(T));
		_data[--_len] = 0;
	}

	void insert_after(int idx, slice<const T> s) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (idx == _len - 1) {
			append(s);
		} else {
			insert_before(idx+1, s);
		}
	}

	void insert_after(int idx, slice<T> s) {
		insert_after(idx, slice<const T>(s));
	}

	void insert_before(int idx, slice<const T> s) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (_len + s.len() > _cap) {
			reserve(_new_size(_len + s.len()));
		}
		T *src = _data + idx;
		T *dst = src + s.len();
		::memmove(dst, src, (_len - idx) * sizeof(T));
		::memcpy(src, s.data(), s.len() * sizeof(T));
		_len += s.len();
		_data[_len] = 0;

	}

	void insert_before(int idx, slice<T> s) {
		insert_before(idx, slice<const T>(s));
	}

	void append(slice<const T> s) {
		if (_len + s.len() > _cap) {
			reserve(_new_size(_len + s.len()));
		}
		::memcpy(_data + _len, s.data(), s.len() * sizeof(T));
		_len += s.len();
		_data[_len] = 0;
	}

	void append(slice<T> s) {
		append(slice<const T>(s));
	}

	void remove(int begin, int end) {
		_ZBS_ASSERT(begin < end);
		_ZBS_BOUNDS_CHECK(begin, _len);
		_ZBS_BOUNDS_CHECK(end, _len+1);
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
		_ZBS_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}

	slice<T> sub(int begin, int end) {
		_ZBS_ASSERT(begin < end);
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
		_ZBS_ASSERT(begin < end);
		_ZBS_BOUNDS_CHECK(begin, _len);
		_ZBS_BOUNDS_CHECK(end, _len+1);
		return {_data + begin, end - begin};
	}
};

extern template class basic_string<char>;
extern template class basic_string<char16_t>;
extern template class basic_string<char32_t>;

//============================================================================
// utf-8 string
//============================================================================

class string : public basic_string<char> {
public:
	string() = default;
	string(const string&) = default;
	string(string&&) = default;
	~string() = default;
	string &operator=(const string&) = default;
	string &operator=(string&&) = default;

public:
	string(const char *cstr);
	string &operator=(const char *cstr);
	char &operator[](int idx) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}
	const char &operator[](int idx) const {
		_ZBS_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}
};

bool operator==(const string &lhs, const string &rhs);
bool operator==(const char *lhs, const string &rhs);
bool operator==(const string &lhs, const char *rhs);

} // namespace zbs
