#include "stf.hh"
#include "zbs.hh"

STF_SUITE_NAME("zbs/slice");

STF_TEST("slice copy") {
	{
		zbs::string a = "hello";
		zbs::string b = "world";
		zbs::copy(a.sub(), b.sub());
		STF_ASSERT(a == "world");
		STF_ASSERT(b == "world");
	}
	{
		zbs::string a = "111111222222333333";
		zbs::copy(a.sub(3), a.sub(a.len()-6));
		STF_ASSERT(a == "111333333222333333");
	}
	{
		zbs::string a = "111111222222333333";
		zbs::copy(a.sub(a.len()-9), a.sub(3, 9));
		STF_ASSERT(a == "111111222111222333");
	}
	// TODO: test non-POD type
}
