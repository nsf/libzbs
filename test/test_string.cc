#include "stf.hh"
#include "zbs/string.hh"
#include <utility>

STF_SUITE_NAME("zbs/string");

STF_TEST("string::string()") {
	// default ctor
	zbs::string a;
	STF_ASSERT(a.len() == 0);
	STF_ASSERT(a.cap() == 0);
	STF_ASSERT(a.data() != nullptr);
	STF_ASSERT(a == "");
}

STF_TEST("string::string(const char*)") {
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

STF_TEST("string::string(const string&)") {
	// copy ctor
	zbs::string a = "test";
	zbs::string b = a;
	STF_ASSERT(a == "test");
	STF_ASSERT(b == "test");
}

STF_TEST("string::string(string&)") {
	// move ctor
	zbs::string a = "check";
	zbs::string b = std::move(a);
	STF_ASSERT(a == "");
	STF_ASSERT(a.cap() == 0);
	STF_ASSERT(b == "check");
}

STF_TEST("string::operator=(slice<const char>)") {
	// slice operator=
	zbs::string a = "1234";
	a = a.sub(0, 2);
	STF_ASSERT(a == "12");

	zbs::string b = "7890";
	b = b.sub(1, 3);
	STF_ASSERT(b == "89");
}

STF_TEST("string::operator=(const char*)") {
	// c-string operator=
	zbs::string a = "123456";
	a = "123";
	STF_ASSERT(a == "123");
	STF_ASSERT(a.cap() >= 6);

	a = "";
	STF_ASSERT(a == "");
	STF_ASSERT(a.cap() >= 6);

	zbs::string b;
	b = b;
	b = "";
	STF_ASSERT(b == "");
	STF_ASSERT(b.cap() == 0);
	STF_ASSERT(b.data() == zbs::detail::char_traits<char>::empty_string());

	b = "hello, world";
	STF_ASSERT(b == "hello, world");
	b = b.sub(b.len() - 5);
	STF_ASSERT(b == "world");
}

STF_TEST("string::operator=(const string&)") {
	// copy operator=
	zbs::string a = "123456";
	zbs::string b = "123";
	a = b;
	STF_ASSERT(a == "123");
	STF_ASSERT(a.cap() >= 6);
	STF_ASSERT(b == "123");

	zbs::string c = ":)";
	c = c;
	STF_ASSERT(c == ":)");
}

STF_TEST("string::operator=(string&)") {
	// move operator=
	zbs::string a = "123";
	zbs::string b = "abc";
	a = std::move(b);
	STF_ASSERT(a == "abc");
	STF_ASSERT(b == "");
}

STF_TEST("string::clear()") {
	// clear
	zbs::string a = "123";
	a.clear();
	STF_ASSERT(a == "");
	STF_ASSERT(a.cap() == 3);
}

STF_TEST("string::reserve(int)") {
	// reserve
	zbs::string a;
	a.reserve(80);
	STF_ASSERT(a == "");
	STF_ASSERT(a.cap() >= 80);

	zbs::string b = "hello";
	b.reserve(80);
	STF_ASSERT(b == "hello");
	STF_ASSERT(b.cap() >= 80);

	zbs::string c;
	c.reserve(0);
	STF_ASSERT(c == "");
	STF_ASSERT(c.cap() == 0);
	STF_ASSERT(c.data() == zbs::detail::char_traits<char>::empty_string());

	c.reserve(1);
	STF_ASSERT(c == "");
	STF_ASSERT(c.cap() >= 1);
	STF_ASSERT(c.data() != zbs::detail::char_traits<char>::empty_string());
}

STF_TEST("string::shrink()") {
	// shrink
	zbs::string a = "0000000000000000000000000000000000000";
	a = "123";
	a.shrink();
	STF_ASSERT(a == "123");
	STF_ASSERT(a.cap() == 3);

	zbs::string b;
	b.shrink();
	STF_ASSERT(b.cap() == 0);
	STF_ASSERT(b == "");
	STF_ASSERT(b.data() == zbs::detail::char_traits<char>::empty_string());
}

STF_TEST("string::resize(int, T)") {
	// resize
	zbs::string a = "123";
	a.resize(20);
	STF_ASSERT(a.sub(0, 3) == "123");
	STF_ASSERT(a.len() == 20);

	zbs::string b = "456";
	b.resize(6, '-');
	STF_ASSERT(b == "456---");

	b.resize(0);
	STF_ASSERT(b == "");

	zbs::string c;
	c.resize(0);
	STF_ASSERT(c == "");
	STF_ASSERT(c.cap() == 0);
	STF_ASSERT(c.data() == zbs::detail::char_traits<char>::empty_string());
}

STF_TEST("string::insert(int, slice<const char>)") {
	// insert
	zbs::string a = "123";
	a.insert(0, 'a');
	STF_ASSERT(a == "a123");
	a.insert(3, 'b');
	STF_ASSERT(a == "a12b3");
	a.insert(1, 'c');
	STF_ASSERT(a == "ac12b3");
	a.insert(a.len(), 'x');
	STF_ASSERT(a == "ac12b3x");

	zbs::string b = "3";
	b.insert(0, "12");
	STF_ASSERT(b == "123");
	b.insert(2, a.sub());
	STF_ASSERT(b == "12ac12b3x3");
	b.insert(2, "123");
	STF_ASSERT(b == "12123ac12b3x3");

	zbs::string c = "aabbcc";
	c.insert(0, c.sub(0, 2));
	STF_ASSERT(c == "aaaabbcc");
	c.insert(c.len(), c.sub(0, 2));
	STF_ASSERT(c == "aaaabbccaa");
	c.insert(4, c.sub(2, 6));
	STF_ASSERT(c == "aaaaaabbbbccaa");
}

STF_TEST("string::append(slice<const char>)") {
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

	b.append(b);
	STF_ASSERT(b == "hello hellohello hello");
	b.append(b.sub(b.len()-1));
	STF_ASSERT(b == "hello hellohello helloo");
}

STF_TEST("string::remove(int, int)") {
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
