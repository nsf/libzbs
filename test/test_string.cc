#include "stf.hh"
#include "zbs/string.hh"
#include <utility>

STF_SUITE_NAME("zbs/string");

STF_TEST("string ctor/dtor and ops") {
	{
		// default ctor
		zbs::string a;
		STF_ASSERT(a.len() == 0);
		STF_ASSERT(a.cap() == 0);
		STF_ASSERT(a.data() != nullptr);
		STF_ASSERT(a == "");
	}
	{
		// c-string ctor
		zbs::string a = "hello, world";
		STF_ASSERT(a.len() == 12);
		STF_ASSERT(a.cap() >= 12);
		STF_ASSERT(a == "hello, world");

		zbs::string b = u8"123";
		STF_ASSERT(b.len() == 3);
		STF_ASSERT(b.cap() >= 3);
		STF_ASSERT(b == "123");
	}
	{
		// copy ctor
		zbs::string a = "test";
		zbs::string b = a;
		STF_ASSERT(a == "test");
		STF_ASSERT(b == "test");
	}
	{
		// move ctor
		zbs::string a = "check";
		zbs::string b = std::move(a);
		STF_ASSERT(a == "");
		STF_ASSERT(a.cap() == 0);
		STF_ASSERT(b == "check");
	}
	{
		// c-string operator=
		zbs::string a = "123456";
		a = "123";
		STF_ASSERT(a == "123");
		STF_ASSERT(a.cap() >= 6);
	}
	{
		// copy operator=
		zbs::string a = "123456";
		zbs::string b = "123";
		a = b;
		STF_ASSERT(a == "123");
		STF_ASSERT(a.cap() >= 6);
		STF_ASSERT(b == "123");
	}
	{
		// move operator=
		zbs::string a = "123";
		zbs::string b = "abc";
		a = std::move(b);
		STF_ASSERT(a == "abc");
		STF_ASSERT(b == "");
	}
}

STF_TEST("string methods") {
	{
		// clear
		zbs::string a = "123";
		a.clear();
		STF_ASSERT(a == "");
		STF_ASSERT(a.cap() == 3);
	}
	{
		// reserve
		zbs::string a;
		a.reserve(80);
		STF_ASSERT(a == "");
		STF_ASSERT(a.cap() >= 80);

		zbs::string b = "hello";
		b.reserve(80);
		STF_ASSERT(b == "hello");
		STF_ASSERT(b.cap() >= 80);
	}
	{
		// shrink
		zbs::string a = "0000000000000000000000000000000000000";
		a = "123";
		a.shrink();
		STF_ASSERT(a == "123");
		STF_ASSERT(a.cap() == 3);
	}
	{
		// resize
		zbs::string a = "123";
		a.resize(20);
		STF_ASSERT(a == "123");
		STF_ASSERT(a.len() == 20);

		zbs::string b = "456";
		b.resize(6, '-');
		STF_ASSERT(b == "456---");
	}
	{
		// insert_after
		zbs::string a = "123";
		a.insert_after(0, 'a');
		STF_ASSERT(a == "1a23");
		a.insert_after(3, 'b');
		STF_ASSERT(a == "1a23b");
		a.insert_after(1, 'c');
		STF_ASSERT(a == "1ac23b");

		zbs::string b = "1";
		b.insert_after(0, "23");
		STF_ASSERT(b == "123");
		b.insert_after(0, a.sub(1, 3));
		STF_ASSERT(b == "1ac23");
		b.insert_after(2, a.sub(5));
		STF_ASSERT(b == "1acb23");
	}
	{
		// insert_before
		zbs::string a = "123";
		a.insert_before(0, 'a');
		STF_ASSERT(a == "a123");
		a.insert_before(3, 'b');
		STF_ASSERT(a == "a12b3");
		a.insert_before(1, 'c');
		STF_ASSERT(a == "ac12b3");

		zbs::string b = "3";
		b.insert_before(0, "12");
		STF_ASSERT(b == "123");
		b.insert_before(2, a.sub());
		STF_ASSERT(b == "12ac12b33");
		b.insert_before(2, "123");
		STF_ASSERT(b == "12123ac12b33");
	}
	{
		// append
		zbs::string a;
		a.append('h');
		a.append('e');
		a.append('l');
		a.append('l');
		a.append('o');
		STF_ASSERT(a == "hello");

		zbs::string b;
		b.append(a.sub());
		b.append(" ");
		b.append(a.sub());
		STF_ASSERT(b == "hello hello");
	}
	{
		// remove
		zbs::string a = "12345";
		a.remove(0);
		STF_ASSERT(a == "2345");
		a.remove(3);
		STF_ASSERT(a == "234");
		a.remove(1);
		STF_ASSERT(a == "24");

		zbs::string b = "123456789";
		b.remove(0, 2);
		STF_ASSERT(b == "3456789");
		b.remove(2, 4);
		STF_ASSERT(b == "34789");
		b.remove(3, 5);
		STF_ASSERT(b == "347");
	}
}
