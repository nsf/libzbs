#pragma once

#include <new>
#include <utility>
#include <type_traits>
#include "_utils.hh"

namespace zbs {

struct nullopt_t {};
constexpr nullopt_t nullopt{};

struct in_place_t {};
constexpr in_place_t in_place{};

template <typename T>
class optional {
	union {
		char noval;
		T val;
	};
	bool valid = false;

public:
	constexpr optional(): noval(0) {}
	constexpr optional(nullopt_t): noval(0) {}
	optional(const optional &r): valid(r.valid) {
		if (valid)
			new (&val) T(r.val);
	}
	optional(optional &&r): valid(r.valid) {
		if (valid)
			new (&val) T(std::move(r.val));
	}
	constexpr optional(const T &r): val(r), valid(true) {}
	constexpr optional(T &&r): val(std::move(r)), valid(true) {}

	template <typename ...Args>
	constexpr explicit optional(in_place_t, Args &&...args):
		val(std::forward<Args>(args)...), valid(true) {}

	~optional() { if (valid) val.~T(); }

	optional &operator=(nullopt_t) {
		if (valid) {
			val.~T();
			valid = false;
		}
		return *this;
	}

	optional &operator=(const optional &r) {
		if (valid == r.valid) {
			if (valid)
				val = r.val;
		} else {
			if (valid) {
				// valid == true, r.valid == false
				val.~T();
			} else {
				// valid == false, r.valid == true
				new (&val) T(r.val);
			}
			valid = r.valid;
		}
		return *this;
	}

	optional &operator=(optional &&r) {
		if (valid == r.valid) {
			if (valid)
				val = std::move(r.val);
		} else {
			if (valid) {
				// valid == true, r.valid == false
				val.~T();
			} else {
				// valid == false, r.valid == true
				new (&val) T(std::move(r.val));
			}
			valid = r.valid;
		}
		return *this;
	}

	template <typename U, typename = typename std::enable_if<
		std::is_same<typename std::remove_reference<U>::type, T>::value
	>::type>
	optional &operator=(U &&r) {
		if (valid) {
			val = std::forward<U>(r);
		} else {
			new (&val) T(std::forward<U>(r));
			valid = true;
		}
		return *this;
	}

	const T *operator->() const { _ZBS_ASSERT(valid); return &val; }
	const T &operator*() const { _ZBS_ASSERT(valid); return val; }
	T *operator->() { _ZBS_ASSERT(valid); return &val; }
	T &operator*() { _ZBS_ASSERT(valid); return val; }

	template <typename U>
	T value_or(U &&v) const& {
		return valid ? val : static_cast<T>(std::forward<U>(v));
	}
	template <typename U>
	T value_or(U &&v) && {
		return valid ? std::move(val) : static_cast<T>(std::forward<U>(v));
	}

	constexpr explicit operator bool() const { return valid; }
};

template <typename T>
bool operator==(const optional<T> &lhs, const optional<T> &rhs) {
	if (static_cast<bool>(lhs) != static_cast<bool>(rhs))
		return false;
	if (!static_cast<bool>(lhs))
		return true;
	return *lhs == *rhs;
}

template <typename T>
bool operator<(const optional<T> &lhs, const optional<T> &rhs) {
	if (!static_cast<bool>(rhs))
		return false;
	if (!static_cast<bool>(lhs))
		return true;
	return *lhs < *rhs;
}

template <typename T>
bool operator==(const optional<T> &lhs, nullopt_t) {
	return !static_cast<bool>(lhs);
}

template <typename T>
bool operator==(nullopt_t, const optional<T> &rhs) {
	return !static_cast<bool>(rhs);
}

template <typename T>
bool operator<(const optional<T>&, nullopt_t) {
	return false;
}

template <typename T>
bool operator<(nullopt_t, const optional<T> &rhs) {
	return static_cast<bool>(rhs);
}

template <typename T>
bool operator==(const optional<T> &lhs, const T &rhs) {
	return static_cast<bool>(lhs) ? *lhs == rhs : false;
}

template <typename T>
bool operator==(const T &lhs, const optional<T> &rhs) {
	return static_cast<bool>(rhs) ? lhs == *rhs : false;
}

template <typename T>
bool operator<(const optional<T> &lhs, const T &rhs) {
	return static_cast<bool>(lhs) ? *lhs < rhs : true;
}

template <typename T>
bool operator<(const T &lhs, const optional<T> &rhs) {
	return static_cast<bool>(rhs) ? lhs < *rhs : false;
}

template <typename T>
constexpr optional<typename std::decay<T>::type> make_optional(T &&v) {
	return optional<typename std::decay<T>::type>(std::forward<T>(v));
}

} // namespace zbs
