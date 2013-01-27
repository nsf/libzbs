#pragma once

#include <initializer_list>
#include <limits>
#include <memory>
#include <algorithm>
#include <cstdlib>

#include "error.hh"
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

	int _new_size(int requested) const {
		const int max = std::numeric_limits<int>::max();
		if (_cap > max) {
			return max;
		}
		return std::max(_cap * 2, requested);
	}

	// expects: idx < _len, idx >= 0, offset > 0
	void _move_forward(int idx, int offset) {
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
	void _move_backward(int idx, int offset) {
		int src = idx;
		int dst = idx+offset;
		while (src < _len) {
			new (&_data[dst]) T(std::move(_data[src]));
			_data[src].~T();
			src++;
			dst++;
		}
	}

public:
	vector(): _data(nullptr), _len(0), _cap(0) {}

	vector(std::initializer_list<T> r): _len(r.size()), _cap(r.size()) {
		_data = static_cast<T*>(malloc(_cap * sizeof(T)));
		auto it = r.begin();
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(*it++);
		}
	}

	vector(const vector &r): _len(r._len), _cap(r._len) {
		_data = static_cast<T*>(malloc(_cap * sizeof(T)));
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(r._data[i]);
		}
	}

	vector(vector &&r): _data(r._data), _len(r._len), _cap(r._cap) {
		r._data = nullptr;
		r._len = 0;
		r._cap = 0;
	}

	vector(slice<const T> r): _len(r.len()), _cap(r.len()) {
		_data = static_cast<T*>(malloc(_cap * sizeof(T)));
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(r.data()[i]);
		}
	}

	vector(slice<T> r): vector(slice<const T>(r)) {}

	vector &operator=(std::initializer_list<T> r) {
		for (int i = 0; i < _len; i++) {
			_data[i].~T();
		}
		if (_cap < r.size()) {
			free(static_cast<void*>(_data));
			_cap = r.size();
			_data = static_cast<T*>(malloc(_cap * sizeof(T)));
		}
		_len = r.size();
		auto it = r.begin();
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(*it++);
		}
		return *this;
	}

	vector &operator=(const vector &r) {
		for (int i = 0; i < _len; i++) {
			_data[i].~T();
		}
		if (_cap < r._len) {
			free(static_cast<void*>(_data));
			_cap = r._len;
			_data = static_cast<T*>(malloc(_cap * sizeof(T)));
		}
		_len = r._len;
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(r._data[i]);
		}
		return *this;
	}

	vector &operator=(vector &&r) {
		for (int i = 0; i < _len; i++) {
			_data[i].~T();
		}
		free(static_cast<void*>(_data));
		_data = r._data;
		_len = r._len;
		_cap = r._cap;
		r._data = nullptr;
		r._len = 0;
		r._cap = 0;
		return *this;
	}

	int len() const { return _len; }
	int cap() const { return _cap; }
	T *data() { return _data; }
	const T *data() const { return _data; }

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
		free(static_cast<void*>(old_data));
	}

	void shrink() {
		if (_cap == _len) {
			return;
		}

		T *old_data = _data;
		_cap = _len;
		_data = static_cast<T*>(malloc(_cap * sizeof(T)));
		for (int i = 0; i < _len; i++) {
			new (&_data[i]) T(std::move(old_data[i]));
			old_data[i].~T();
		}
		free(static_cast<void*>(old_data));
	}

	void resize(int n) {
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
			new (&_data[i]) T();
		}
		_len = n;
	}

	void resize(int n, const T &elem) {
		// copy & paste from resize(int)
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
	void insert_after(int idx, Args &&...args) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (idx == _len - 1) {
			append(std::forward<Args>(args)...);
		} else {
			insert_before(idx+1, std::forward<Args>(args)...);
		}
	}

	template <typename ...Args>
	void insert_before(int idx, Args &&...args) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (_len + 1 > _cap) {
			reserve(_new_size(_len + 1));
		}
		_move_forward(idx, 1);
		new (&_data[idx]) T(std::forward<Args>(args)...);
		_len++;
	}

	template <typename ...Args>
	void append(Args &&...args) {
		if (_len + 1 > _cap) {
			reserve(_new_size(_len + 1));
		}

		new (&_data[_len]) T(std::forward<Args>(args)...);
		_len++;
	}

	void remove(int idx) {
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

	void insert_after(int idx, slice<const T> s) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (idx == _len - 1) {
			append(s);
		} else {
			insert_before(idx+1, s);
		}
	}

	void insert_after(int idx, std::initializer_list<T> r) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (idx == _len - 1) {
			append(r);
		} else {
			insert_before(idx + 1, r);
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
		_move_forward(idx, s.len());
		for (int i = 0; i < s.len(); i++) {
			new (&_data[idx+i]) T(s.data()[i]);
		}
		_len += s.len();
	}

	void insert_before(int idx, std::initializer_list<T> r) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		if (_len + r.size() > _cap) {
			reserve(_new_size(_len + r.size()));
		}
		_move_forward(idx, r.size());

		auto it = r.begin();
		for (int i = 0; i < r.size(); i++) {
			new (&_data[idx + i]) T(*it++);
		}
		_len += r.size();
	}

	void insert_before(int idx, slice<T> s) {
		insert_before(idx, slice<const T>(s));
	}

	void append(slice<const T> s) {
		if (_len + s.len() > _cap) {
			reserve(_new_size(_len + s.len()));
		}

		for (int i = 0; i < s.len(); i++) {
			new (&_data[_len + i]) T(s.data()[i]);
		}
		_len += s.len();
	}

	void append(std::initializer_list<T> r) {
		if (_len + r.size() > _cap) {
			reserve(_new_size(_len + r.size()));
		}

		auto it = r.begin();
		for (int i = 0; i < r.size(); i++) {
			new (&_data[_len + i]) T(*it++);
		}
		_len += r.size();
	}

	void append(slice<T> s) {
		append(slice<const T>(s));
	}

	void remove(int begin, int end) {
		_ZBS_ASSERT(begin < end);
		_ZBS_BOUNDS_CHECK(begin, _len);
		_ZBS_BOUNDS_CHECK(end, _len+1);
		const int len = end - begin;
		for (int i = begin; i < end; i++) {
			_data[i].~T();
		}

		if (end < _len) {
			_move_backward(begin+len, -len);
		}
		_len -= len;
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

	T &operator[](int idx) {
		_ZBS_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}

	const T &operator[](int idx) const {
		_ZBS_BOUNDS_CHECK(idx, _len);
		return _data[idx];
	}

	operator slice<T>() { return {_data, _len}; }
	operator slice<const T>() const { return {_data, _len}; }
};

} // namespace zbs
