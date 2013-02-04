#pragma once

#include <initializer_list>
#include <limits>
#include <memory>
#include <algorithm>
#include <cstdlib>

#include "_utils.hh"
#include "slice.hh"

namespace zbs {

// A typical std::vector clone, but as you may have noticed, it has no
// customizable allocator. The reason for this is the following. From what I've
// seen the purpose of having a custom allocator is performance considerations
// and/or memory fragmentation/alignment issues. Surprisingly enough we may
// discover papers like the one about EASTL which quickly conclude that STL
// allocators have a lot of weak points and it makes them useless in hardware
// environments like gaming console. Today (early 2013) it seems that future
// hardware will move towards amd64 (aka x86_64) architecture with virtual
// memory and such. By rumors future Sony console will use AMD fusion
// processors for example. And even today having Intel Ivy Bridge CPU you can
// see how better this hardware is compared to other CPUs on the market. Memory
// becomes really cheap too. Having all that and things I haven't mentioned in
// mind the idea is that 98% of users will not use custom allocators because
// they don't care. The rest 2% are free to implement their own containers, and
// that's what most likely they do already anyway.
//
// vector does not provide exception-safety guarantees

//============================================================================
// vector template
//============================================================================
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
	vector(): _data(nullptr), _len(0), _cap(0) {}

	vector(std::initializer_list<T> r): vector(slice<const T>(r)) {}

	vector(const vector &r): vector(slice<const T>(r)) {}

	vector(vector &&r): _data(r._data), _len(r._len), _cap(r._cap) {
		r._data = nullptr;
		r._len = 0;
		r._cap = 0;
	}

	vector(slice<const T> r): _data(nullptr), _len(r.len()), _cap(r.len()) {
		if (_len == 0) {
			return;
		}
		_data = static_cast<T*>(malloc(_cap * sizeof(T)));
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(r.data()[i]);
		}
	}

	vector(slice<T> r): vector(slice<const T>(r)) {}

	explicit vector(int n): _data(nullptr), _len(n), _cap(n) {
		_ZBS_ASSERT(n >= 0);
		if (_len == 0) {
			return;
		}
		_data = static_cast<T*>(malloc(_cap * sizeof(T)));
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T;
		}
	}

	vector(int n, const T &elem): _data(nullptr), _len(n), _cap(n) {
		_ZBS_ASSERT(n >= 0);
		if (_len == 0) {
			return;
		}
		_data = static_cast<T*>(malloc(_cap * sizeof(T)));
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(elem);
		}
	}

	~vector() {
		for (int i = 0; i < _len; i++) {
			_data[i].~T();
		}
		free(_data);
	}

	vector &operator=(slice<const T> r) {
		if (r.data() == _data) {
			// self copy shortcut (a = a)
			return *this;
		}
		if (r.len() > _cap) {
			// slice is bigger than we are, realloc needed, also it
			// means slice cannot point to ourselves and it is save
			// to destroy ourselves
			for (int i = 0; i < _len; i++) {
				_data[i].~T();
			}
			free(_data);
			_cap = _len = r.len();
			_data = static_cast<T*>(malloc(_cap * sizeof(T)));
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

	vector &operator=(slice<T> r) {
		return operator=(slice<const T>(r));
	}

	vector &operator=(std::initializer_list<T> r) {
		return operator=(slice<const T>(r));
	}

	vector &operator=(const vector &r) {
		return operator=(slice<const T>(r));
	}

	vector &operator=(vector &&r) {
		if (&r == this) {
			return *this;
		}
		for (int i = 0; i < _len; i++) {
			_data[i].~T();
		}
		free(_data);
		_data = r._data;
		_len = r._len;
		_cap = r._cap;
		r._data = nullptr;
		r._len = 0;
		r._cap = 0;
		return *this;
	}

	inline int len() const { return _len; }
	inline int cap() const { return _cap; }
	inline T *data() { return _data; }
	inline const T *data() const { return _data; }

	void clear() {
		for (int i = 0; i < _len; i++) {
			_data[i].~T();
		}
		_len = 0;
	}

	void reserve(int n) {
		if (_cap >= n) {
			return;
		}

		T *old_data = _data;
		_cap = n;
		_data = static_cast<T*>(malloc(_cap * sizeof(T)));
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(std::move(old_data[i]));
			old_data[i].~T();
		}
		free(old_data);
	}

	void shrink() {
		if (_cap == _len) {
			return;
		}

		T *old_data = _data;
		_cap = _len;
		if (_len > 0) {
			_data = static_cast<T*>(malloc(_cap * sizeof(T)));
			for (int i = 0; i < _len; i++) {
				new (&_data[i]) T(std::move(old_data[i]));
				old_data[i].~T();
			}
		} else {
			_data = nullptr;
		}
		free(old_data);
	}

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

	template <typename ...Args>
	inline void pinsert(int idx, Args &&...args) {
		_ZBS_BOUNDS_CHECK(idx, _len+1);
		_ensure_capacity(1);
		if (idx < _len) {
			_move_forward(idx, 1);
		}
		new (&_data[idx]) T(std::forward<Args>(args)...);
		_len++;
	}

	template <typename ...Args>
	inline void pappend(Args &&...args) {
		_ensure_capacity(1);
		new (&_data[_len]) T(std::forward<Args>(args)...);
		_len++;
	}

	inline void remove(int idx) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (idx == _len - 1) {
			_data[_len-1].~T();
			_len--;
			return;
		}

		_data[idx].~T();
		_move_backward(idx+1, -1);
		_len--;
	}

	inline void insert(int idx, slice<const T> s) {
		_ZBS_BOUNDS_CHECK(idx, _len+1);
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

	inline void insert(int idx, const T &elem) {
		pinsert(idx, elem);
	}

	inline void insert(int idx, T &&elem) {
		pinsert(idx, elem);
	}

	inline void append(slice<const T> s) {
		insert(_len, s);
	}

	inline void append(const T &elem) {
		pappend(elem);
	}

	inline void append(T &&elem) {
		pappend(elem);
	}

	inline void remove(int begin, int end) {
		_ZBS_ASSERT(begin <= end);
		_ZBS_BOUNDS_CHECK(begin, _len);
		_ZBS_BOUNDS_CHECK(end, _len+1);
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

	inline slice<T> sub() {
		return {_data, _len};
	}

	inline slice<T> sub(int begin) {
		_ZBS_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}

	inline slice<T> sub(int begin, int end) {
		_ZBS_ASSERT(begin <= end);
		_ZBS_BOUNDS_CHECK(begin, _len);
		_ZBS_BOUNDS_CHECK(end, _len+1);
		return {_data + begin, end - begin};
	}

	inline slice<const T> sub() const {
		return {_data, _len};
	}

	inline slice<const T> sub(int begin) const {
		_ZBS_BOUNDS_CHECK(begin, _len);
		return {_data + begin, _len - begin};
	}

	inline slice<const T> sub(int begin, int end) const {
		_ZBS_ASSERT(begin <= end);
		_ZBS_BOUNDS_CHECK(begin, _len);
		_ZBS_BOUNDS_CHECK(end, _len+1);
		return {_data + begin, end - begin};
	}

	inline T &operator[](int idx) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}

	inline const T &operator[](int idx) const {
		_ZBS_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}

	inline operator slice<T>() { return {_data, _len}; }
	inline operator slice<const T>() const { return {_data, _len}; }
};

} // namespace zbs
