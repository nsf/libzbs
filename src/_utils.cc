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

}} // namespace zbs::detail
