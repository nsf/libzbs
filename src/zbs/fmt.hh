#pragma once

#include <type_traits>
#include <cstdio>
#include "_error.hh"
#include "_string.hh"

namespace zbs {
namespace detail {

template <typename T>
struct is_error_ptr :
	public std::integral_constant<bool,
		std::is_pointer<T>::value &&
		std::is_base_of<
			error,
			typename std::remove_pointer<T>::type
		>::value
	> {};

template <typename T, bool OK = is_error_ptr<T>::value>
struct error_ptr_or_null;

template <typename T>
struct error_ptr_or_null<T, true> {
	static constexpr error *get(T &&v) { return v; }
};

template <typename T>
struct error_ptr_or_null<T, false> {
	static constexpr error *get(T&&) { return nullptr; }
};

constexpr error *get_last_if_error() {
	return nullptr;
}

template <typename T>
constexpr error *get_last_if_error(T &&last) {
	return error_ptr_or_null<T>::get(std::forward<T>(last));
}

template <typename T, typename ...Args>
constexpr error *get_last_if_error(T&&, Args &&...other) {
	return get_last_if_error(std::forward<Args>(other)...);
}

} // namespace zbs::detail

namespace fmt {

string sprintf(const char *format, ...);
string vsprintf(const char *format, va_list vl);

// TODO:
//int printf(const char *format, ...);
//int fprintf(io::writer &w, const char *format, ...);

}} // namespace zbs::fmt
