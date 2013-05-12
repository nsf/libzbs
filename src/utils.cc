#include "zbs/_utils.hh"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

namespace zbs {
namespace detail {

void assert_abort(const char *assertion, const char *file, int line, const char *func) {
	fprintf(stderr, "%s:%d: %s: assertion `%s` failed\n", file, line, func, assertion);
	abort();
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
		fprintf(stderr, "memory allocation failure\n");
		abort();
	}
	return mem;
}

void xfree(void *ptr) {
	::free(ptr);
}

}} // namespace zbs::detail
