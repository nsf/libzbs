#pragma once

#include <utility>

namespace zbs {

template <typename T> class func;

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
