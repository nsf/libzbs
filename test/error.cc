#include "stf.hh"
#include "zbs.hh"
#include <cstring>

STF_SUITE_NAME("zbs::error");

STF_TEST("random") {
	using namespace zbs;
	error a {error_verbosity::quiet};
	a.set(zbs::generic_error_code, "oops, something weird happened");
	STF_ASSERT(a.code() == zbs::generic_error_code);
	STF_ASSERT(std::strcmp(a.what(), "") == 0);
	STF_ASSERT(a);

	error b {error_verbosity::quiet};
	STF_ASSERT(!b);

	error c {error_verbosity::verbose};
	c.set(zbs::generic_error_code, "oops, I did it again: %d", 43);
	STF_ASSERT(c.code() == zbs::generic_error_code);
	STF_ASSERT(std::strcmp(c.what(), "oops, I did it again: 43") == 0);
	STF_ASSERT(c);
}
