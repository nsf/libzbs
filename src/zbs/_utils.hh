#pragma once

#include <type_traits>
#include "_config.h"
#include "types.hh"

#ifdef ZBS_ENABLE_ASSERT
	#define _ZBS_ASSERT(expr)				\
	do {							\
		if (!(expr)) {					\
			zbs::detail::assert_abort(#expr,	\
				__FILE__, __LINE__,		\
				__PRETTY_FUNCTION__);		\
		}						\
	} while (0)
#else
	#define _ZBS_ASSERT(expr) ((void)0)
#endif

#define _ZBS_SLICE_BOUNDS_CHECK(index, length) \
	_ZBS_ASSERT((unsigned int)(index) <= (unsigned int)(length))

#define _ZBS_IDX_BOUNDS_CHECK(index, length) \
	_ZBS_ASSERT((unsigned int)(index) < (unsigned int)(length))

#define _ZBS_ASSERT_IS_SAME_DISREGARDING_CONST(...) \
	static_assert(zbs::detail::is_same_disregarding_const<__VA_ARGS__>::value, \
		"types must be the same (disregrading const type qualifier)")

namespace zbs {
namespace detail {

template <typename ...Args>
struct is_same_disregarding_const;

template <typename T>
struct is_same_disregarding_const<T> : public std::true_type {};

template <typename T, typename U, typename ...Args>
struct is_same_disregarding_const<T, U, Args...> :
	public std::integral_constant<bool,
		std::is_same<
			typename std::remove_const<T>::type,
			typename std::remove_const<U>::type
		>::value &&
		is_same_disregarding_const<U, Args...>::value
	> {};

void assert_abort(const char *assertion, const char *file, int line, const char *func);
uint32 fastrand();
void *xmalloc(int n);
void xfree(void *ptr);

template <typename T> T *malloc(int n) { return (T*)xmalloc(sizeof(T) * n); }
template <typename T> void free(T *ptr) { xfree(ptr); }

}} // namespace zbs::detail
