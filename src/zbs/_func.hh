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
		auto obj = static_cast<T*>(data);
		return (*obj)(std::forward<Args>(args)...);
	}

	template <typename T>
	static R _invoke_const_obj(void *data, Args &&...args) {
		auto obj = static_cast<const T*>(data);
		return (*obj)(std::forward<Args>(args)...);
	}

	R (*_invoker)(void*, Args&&...) {nullptr};
	void *_data {nullptr};

public:
	func() = default;
	func(const func &r) = default;
	func(func &&r) = default;

	// We need this little guy here, because otherwise template ctor
	// func(T &obj) will be considered as a better overload match
	// in some cases
	func(func &r) = default;

	template <typename T>
	func(T &obj): _invoker(_invoke_obj<T>),
		_data(static_cast<void*>(&obj)) {}
	template <typename T>
	func(const T &obj): _invoker(_invoke_const_obj<T>),
		_data(static_cast<void*>(const_cast<T*>(&obj))) {}
	func(R (*fp)(Args...)): _invoker(_invoke_func),
		_data(reinterpret_cast<void*>(fp)) {}

	func &operator=(const func&) = default;
	func &operator=(func&&) = default;
	R operator()(Args ...args) const {
		return (*_invoker)(_data, std::forward<Args>(args)...);
	}

	explicit operator bool() const { return _data != nullptr; }
};

} // namespace zbs
