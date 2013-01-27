#include "zbs/error.hh"
#include <cstdio>
#include <cstdlib>

namespace zbs {

void _assert_abort(const char *assertion, const char *file, int line, const char *func) {
	fprintf(stderr, "%s:%d: %s: assertion `%s` failed\n", file, line, func, assertion);
	abort();
}

} // namespace zbs
