#include "stf.hh"
#include "zbs.hh"
#include <cstring>

STF_SUITE_NAME("zbs::error");

STF_TEST("random") {
	zbs::error a;
	a.set(zbs::generic_error_code, "oops, something weird happened");
	STF_ASSERT(a.code() == zbs::generic_error_code);
	STF_ASSERT(std::strcmp(a.what(), "") == 0);
	STF_ASSERT(a);

	zbs::error b;
	STF_ASSERT(!b);

	zbs::verbose_error c;
	c.set(zbs::generic_error_code, "oops, I did it again: %d", 43);
	STF_ASSERT(c.code() == zbs::generic_error_code);
	STF_ASSERT(std::strcmp(c.what(), "oops, I did it again: 43") == 0);
	STF_ASSERT(c);
}
