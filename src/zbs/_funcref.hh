#pragma once

#include <new>
#include <utility>
#include "_types.hh"

namespace zbs {

template <typename T> class funcref;

// I think at the moment it violates a couple of C++ rules. A non-POD object is
// being constructed, but never destructed (I know the destructor is a no-op,
// but anyway). And a non-POD object is being memcpy'ed without using a copy
// constructor/operator, I assume it's ok to do so, because essentially we only
// copy two pointers here (the vtable pointer and the pointer of one of the
// implementations). There are ways to avoid that, but let's leave it like that
// for now.

template <typename R, typename ...Args>
class funcref<R (Args...)> {
	class _impl {
	public:
		virtual R operator()(Args...) = 0;
	};

	class _func_impl : public _impl {
		R (*_fp)(Args...);

	public:
		constexpr _func_impl(R (*fp)(Args...)): _fp(fp) {}
		R operator()(Args ...args) override {
			return (*_fp)(std::forward<Args>(args)...);
		}
	};

	template <typename T>
	class _object_impl : public _impl {
		T &_obj;

	public:
		constexpr _object_impl(T &obj): _obj(obj) {}
		R operator()(Args ...args) override {
			return _obj(std::forward<Args>(args)...);
		}
	};

	union _sizer {
		_func_impl _1;
		_object_impl<int> _2;
	};

	byte _data[sizeof(_sizer)];
public:
	template <typename T>
	funcref(T &obj) {
		static_assert(sizeof(_object_impl<T>) == sizeof(_object_impl<int>),
			"all T& should have no impact on _object_impl size");
		new (_data) _object_impl<T>(obj);
	}
	template <typename T>
	funcref(const T &obj) {
		static_assert(sizeof(_object_impl<const T>) == sizeof(_object_impl<int>),
			"all T& should have no impact on _object_impl size");
		new (_data) _object_impl<const T>(obj);
	}
	funcref(R (*fp)(Args...)) {
		new (_data) _func_impl(fp);
	}
	R operator()(Args ...args) {
		return (*(_impl*)_data)(std::forward<Args>(args)...);
	}
};

} // namespace zbs
