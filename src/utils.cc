#include "zbs/_utils.hh"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <new>

namespace zbs {
namespace detail {

void assert_abort(const char *assertion, const char *file, int line, const char *func) {
	std::fprintf(stderr, "%s:%d: %s: assertion `%s` failed\n", file, line, func, assertion);
	std::abort();
}

static thread_local uint32 fastrand_state;

uint32 fastrand() {
	uint32 x = fastrand_state;
	x += x;
	if (x & 0x80000000L)
		x ^= 0x88888eefUL;
	fastrand_state = x;
	return x;
}

void *xmalloc(int n) {
	void *mem = ::malloc(n);
	if (mem == nullptr) {
		std::fprintf(stderr, "memory allocation failure\n");
		std::abort();
	}
	return mem;
}

void xfree(void *ptr) {
	::free(ptr);
}

}} // namespace zbs::detail

namespace zbs {

const or_die_t or_die = {};

} // namespace zbs

void *operator new(size_t size, const zbs::or_die_t&) noexcept {
	void *out = operator new(size, std::nothrow);
	if (!out) {
		std::fprintf(stderr, "libzbs: out of memory\n");
		std::abort();
	}
	return out;
}

void *operator new[](size_t size, const zbs::or_die_t&) noexcept {
	void *out = operator new[](size, std::nothrow);
	if (!out) {
		std::fprintf(stderr, "libzbs: out of memory\n");
		std::abort();
	}
	return out;
}
