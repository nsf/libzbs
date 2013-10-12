#pragma once

#include <new>
#include <utility>
#include "_types.hh"

namespace zbs {

template <typename T> class func;

// I think at the moment it violates a couple of C++ rules. A non-POD object is
// being constructed, but never destructed (I know the destructor is a no-op,
// but anyway). And a non-POD object is being memcpy'ed without using a copy
// constructor/operator, I assume it's ok to do so, because essentially we only
// copy two pointers here (the vtable pointer and the pointer of one of the
// implementations). There are ways to avoid that, but let's leave it like that
// for now.

template <typename R, typename ...Args>
class func<R (Args...)> {
	static R _invoke_func(void *data, Args &&...args) {
		auto fp = reinterpret_cast<R (*)(Args...)>(data);
		return (*fp)(std::forward<Args>(args)...);
	}

	template <typename T>
	static R _invoke_obj(void *data, Args &&...args) {
		auto obj = reinterpret_cast<T*>(data);
		return (*obj)(std::forward<Args>(args)...);
	}

	template <typename T>
	static R _invoke_const_obj(void *data, Args &&...args) {
		auto obj = reinterpret_cast<const T*>(data);
		return (*obj)(std::forward<Args>(args)...);
	}

	R (*_invoker)(void*, Args&&...);
	void *_data;

public:
	template <typename T>
	func(T &obj): _invoker(_invoke_obj<T>),
		_data(reinterpret_cast<void*>(&obj)) {}
	template <typename T>
	func(const T &obj): _invoker(_invoke_const_obj<T>),
		_data(reinterpret_cast<void*>(const_cast<T*>(&obj))) {}
	func(R (*fp)(Args...)): _invoker(_invoke_func),
		_data(reinterpret_cast<void*>(fp)) {}
	func(const func&) = default;
	func(func&&) = default;
	func &operator=(const func&) = default;
	func &operator=(func&&) = default;
	R operator()(Args ...args) const {
		return (*_invoker)(_data, std::forward<Args>(args)...);
	}
};

} // namespace zbs
