#include "stf.hh"
#include "zbs/fmt.hh"

using namespace zbs;

STF_SUITE_NAME("zbs/fmt");

STF_TEST("fmt::sprintf(const char*, ...)") {
	int number = 43;
	string s = fmt::sprintf("Hello, %s: %d", "nsf", number);
	STF_ASSERT(s == "Hello, nsf: 43");
}
