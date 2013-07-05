#include "stf.hh"
#include "zbs.hh"

STF_SUITE_NAME("zbs::slice");

using namespace zbs;

STF_TEST("copy(slice<T>, slice<U>)") {
	{
		string a = "hello";
		string b = "world";
		int n = copy(a.sub(), b.sub());
		STF_ASSERT(a == "world");
		STF_ASSERT(b == "world");
		STF_ASSERT(n == 5);
	}
	{
		string a = "111111222222333333";
		int n = copy(a.sub(3), a.sub(a.len()-6));
		STF_ASSERT(a == "111333333222333333");
		STF_ASSERT(n == 6);
	}
	{
		string a = "111111222222333333";
		int n = copy(a.sub(a.len()-9), a.sub(3, 9));
		STF_ASSERT(a == "111111222111222333");
		STF_ASSERT(n == 6);
	}
}

STF_TEST("slice_cast(slice<U>)") {
	int32 a[] = {
		1, 2, 3, 4, 5, 6,
	};
	slice<int32> as = a;
	STF_ASSERT(as.len() == 6);
	STF_ASSERT(as.byte_len() == 24);
	auto bs = slice_cast<int16>(as);
	STF_ASSERT(bs.len() == 12);
	STF_ASSERT(bs.byte_len() == 24);

	int8 c[] = {1, 2, 3};
	slice<int8> cs = c;
	STF_ASSERT(cs.len() == 3);
	STF_ASSERT(cs.byte_len() == 3);
	slice<int32> ds = slice_cast<int32>(cs);
	STF_ASSERT(ds.len() == 0);
	STF_ASSERT(ds.byte_len() == 0);

	int8 e[] = {1, 2, 3, 4, 5};
	slice<int8> es = e;
	STF_ASSERT(es.len() == 5);
	STF_ASSERT(es.byte_len() == 5);
	slice<int32> fs = slice_cast<int32>(es);
	STF_ASSERT(fs.len() == 1);
	STF_ASSERT(fs.byte_len() == 4);
}
