// This file may contain portions of the code derived/adopted from the Go
// standard library, which is covered by a BSD-style license. You can find more
// details in 3rdparty/go_license.txt file.

#include "stf.hh"
#include "zbs.hh"
#include "zbs/strings.hh"
#include "zbs/unicode.hh"
#include "zbs/unicode/utf8.hh"

#include <functional>

STF_SUITE_NAME("zbs::strings");

using namespace zbs;
namespace utf8 = unicode::utf8;

STF_TEST("strings::index(slice<const char>, slice<const char>)") {
	struct index_test {
		string s;
		string sep;
		int out;
	};
	vector<index_test> index_tests = {
		{"", "", 0},
		{"", "a", -1},
		{"", "foo", -1},
		{"fo", "foo", -1},
		{"foo", "foo", 0},
		{"oofofoofooo", "f", 2},
		{"oofofoofooo", "foo", 4},
		{"barfoobarfoo", "foo", 3},
		{"foo", "", 0},
		{"foo", "o", 1},
		{"abcABCabc", "A", 3},
		// cases with one byte strings - test special case in Index()
		{"", "a", -1},
		{"x", "a", -1},
		{"x", "x", 0},
		{"abc", "a", 0},
		{"abc", "b", 1},
		{"abc", "c", 2},
		{"abc", "x", -1},
	};
	for (const auto &test : index_tests) {
		STF_ASSERT(strings::index(test.s, test.sep) == test.out);
	}
}

STF_TEST("strings::index_any(slice<const char>, slice<const char>)") {
	struct index_test {
		string s;
		string sep;
		int out;
	};
	string dots = "1....2....3....4";
	vector<index_test> index_any_tests = {
		{"", "", -1},
		{"", "a", -1},
		{"", "abc", -1},
		{"a", "", -1},
		{"a", "a", 0},
		{"aaa", "a", 0},
		{"abc", "xyz", -1},
		{"abc", "xcz", 2},
		{"a☺b☻c☹d", "uvw☻xyz", 2 + sizeof("☺")-1},
		{"aRegExp*", ".(|)*+?^$[]", 7},
		{dots + dots + dots, " ", -1},
	};
	for (const auto &test : index_any_tests) {
		STF_ASSERT(strings::index_any(test.s, test.sep) == test.out);
	}
}

STF_TEST("strings::index_func(slice<const char>, func<bool(rune)>)") {
	struct index_func_test {
		string in;
		func<bool(rune)> f;
		int first;
		int last;
	};
	auto is_valid_rune = [](rune r) {
		return r != utf8::rune_error;
	};
	auto is_invalid_rune = [](rune r) {
		return r == utf8::rune_error;
	};
	auto is_not_digit = [](rune r) {
		return !unicode::is_digit(r);
	};
	string space = "\t\v\r\f\n\u0085\u00a0\u2000\u3000";
	vector<index_func_test> index_func_tests = {
		{"", is_valid_rune, -1, -1},
		{"abc", unicode::is_digit, -1, -1},
		{"0123", unicode::is_digit, 0, 3},
		{"a1b", unicode::is_digit, 1, 1},
		{space, unicode::is_space, 0, space.len() - 3}, // last rune in space is 3 bytes
		{"\u0e50\u0e5212hello34\u0e50\u0e51", unicode::is_digit, 0, 18},
		{"\u2C6F\u2C6F\u2C6F\u2C6FABCDhelloEF\u2C6F\u2C6FGH\u2C6F\u2C6F", unicode::is_upper, 0, 34},
		{"12\u0e50\u0e52hello34\u0e50\u0e51", is_not_digit, 8, 12},

		// tests of invalid UTF-8
		{"\x80""1", unicode::is_digit, 1, 1},
		{"\x80""abc", unicode::is_digit, -1, -1},
		{"\xc0""a\xc0", is_valid_rune, 1, 1},
		{"\xc0""a\xc0", is_invalid_rune, 0, 2},
		{"\xc0☺\xc0", is_invalid_rune, 0, 4},
		{"\xc0☺\xc0\xc0", is_invalid_rune, 0, 5},
		{"ab\xc0""a\xc0""cd", is_invalid_rune, 2, 4},
		{"a\xe0\x80""cd", is_invalid_rune, 1, 2},
		{"\x80\x80\x80\x80", is_invalid_rune, 0, 3},
	};
	for (const auto &test : index_func_tests) {
		STF_ASSERT(strings::index_func(test.in, test.f) == test.first);
		STF_ASSERT(strings::last_index_func(test.in, test.f) == test.last);
	}
}

STF_TEST("strings::index_rune(slice<const char>, rune)") {
	struct index_rune_test {
		string str;
		rune r;
		int out;
	};
	vector<index_rune_test> index_rune_tests = {
		{"a A x", U'A', 2},
		{"some_text=some_value", U'=', 9},
		{"☺a", U'a', 3},
		{"a☻☺b", U'☺', 4},
	};
	for (const auto &test : index_rune_tests) {
		STF_ASSERT(strings::index_rune(test.str, test.r) == test.out);
	}
}

STF_TEST("strings::join(slice<const string>, slice<const char>)") {
	struct join_test {
		vector<string> a;
		string sep;
		string out;
	};
	vector<join_test> join_tests = {
		{{"1", "2", "3"}, ":", "1:2:3"},
		{{"1", "2", "3"}, "-", "1-2-3"},
		{{"1", "2", "3"}, "", "123"},
		{{}, "::", ""},
		{{}, "", ""},
	};
	for (const auto &test : join_tests) {
		STF_ASSERT(strings::join(test.a, test.sep) == test.out);
	}
}

STF_TEST("strings::last_index(slice<const char>, slice<const char>)") {
	struct index_test {
		string s;
		string sep;
		int out;
	};
	vector<index_test> last_index_tests = {
		{"", "", 0},
		{"", "a", -1},
		{"", "foo", -1},
		{"fo", "foo", -1},
		{"foo", "foo", 0},
		{"foo", "f", 0},
		{"oofofoofooo", "f", 7},
		{"oofofoofooo", "foo", 7},
		{"barfoobarfoo", "foo", 9},
		{"foo", "", 3},
		{"foo", "o", 2},
		{"abcABCabc", "A", 3},
		{"abcABCabc", "a", 6},
	};
	for (const auto &test : last_index_tests) {
		STF_ASSERT(strings::last_index(test.s, test.sep) == test.out);
	}
}

STF_TEST("strings::last_index_any(slice<const char>, slice<const char>)") {
	struct index_test {
		string s;
		string sep;
		int out;
	};
	string dots = "1....2....3....4";
	vector<index_test> last_index_any_tests = {
		{"", "", -1},
		{"", "a", -1},
		{"", "abc", -1},
		{"a", "", -1},
		{"a", "a", 0},
		{"aaa", "a", 2},
		{"abc", "xyz", -1},
		{"abc", "ab", 1},
		{"a☺b☻c☹d", "uvw☻xyz", 2 + sizeof("☺")-1},
		{"a.RegExp*", ".(|)*+?^$[]", 8},
		{dots + dots + dots, " ", -1},
	};
	for (const auto &test : last_index_any_tests) {
		STF_ASSERT(strings::last_index_any(test.s, test.sep) == test.out);
	}
}

string ten_runes(rune ch) {
	char tmp[utf8::utf_max];
	int n = utf8::encode_rune(tmp, ch);
	auto s = slice<char>(tmp).sub(0, n);

	string out;
	for (int i = 0; i < 10; i++) {
		out.append(s);
	}
	return out;
}

rune rot13(rune r) {
	rune step = 13;
	if (r >= 'a' && r <= 'z') {
		return ((r - 'a' + step) % 26) + 'a';
	}
	if (r >= 'A' && r <= 'Z') {
		return ((r - 'A' + step) % 26) + 'A';
	}
	return r;
}

STF_TEST("strings::map(func<rune(rune)>, slice<const char>)") {
	// grow
	auto m = strings::map([](rune) { return unicode::max_rune; }, ten_runes('a'));
	STF_ASSERT(m == ten_runes(unicode::max_rune));

	// shrink
	m = strings::map([](rune) { return 'a'; }, ten_runes(unicode::max_rune));
	STF_ASSERT(m == ten_runes('a'));

	// rot13
	m = strings::map(rot13, "a to zed");
	STF_ASSERT(m == "n gb mrq");

	// rot13^2
	m = strings::map(rot13, strings::map(rot13, "a to zed"));
	STF_ASSERT(m == "a to zed");

	// drop
	m = strings::map([](rune r) {
		return unicode::is(unicode::Latin, r) ? r : -1;
	}, "Hello, 세계");
	STF_ASSERT(m == "Hello");
}

STF_TEST("strings::contains(slice<const char>, slice<const char>)") {
	struct contains_test {
		string str;
		string substr;
		bool expected;
	};
	vector<contains_test> contains_tests = {
		{"abc", "bc", true},
		{"abc", "bcd", false},
		{"abc", "", true},
		{"", "a", false},
	};
	for (const auto &test : contains_tests) {
		STF_ASSERT(strings::contains(test.str, test.substr) == test.expected);
	}
}

STF_TEST("strings::contains_any(slice<const char>, slice<const char>)") {
	struct contains_test {
		string str;
		string substr;
		bool expected;
	};
	string dots = "1....2....3....4";
	vector<contains_test> contains_any_tests = {
		{"", "", false},
		{"", "a", false},
		{"", "abc", false},
		{"a", "", false},
		{"a", "a", true},
		{"aaa", "a", true},
		{"abc", "xyz", false},
		{"abc", "xcz", true},
		{"a☺b☻c☹d", "uvw☻xyz", true},
		{"aRegExp*", ".(|)*+?^$[]", true},
		{dots + dots + dots, " ", false},
	};
	for (const auto &test : contains_any_tests) {
		STF_ASSERT(strings::contains_any(test.str, test.substr) == test.expected);
	}
}

STF_TEST("strings::contains_rune(slice<const char>, rune)") {
	struct contains_rune_test {
		string str;
		rune r;
		bool expected;
	};
	vector<contains_rune_test> contains_rune_tests = {
		{"", U'a', false},
		{"a", U'a', true},
		{"aaa", U'a', true},
		{"abc", U'y', false},
		{"abc", U'c', true},
		{"a☺b☻c☹d", U'x', false},
		{"a☺b☻c☹d", U'☻', true},
		{"aRegExp*", U'*', true},
	};
	for (const auto &test : contains_rune_tests) {
		STF_ASSERT(strings::contains_rune(test.str, test.r) == test.expected);
	}
}

STF_TEST("strings::count(slice<const char>, slice<const char>)") {
	struct count_test {
		string str;
		string sep;
		int count;
	};
	vector<count_test> count_tests = {
		{";;1;;2;3;;4;5;6;7;;", ";;", 4},
		{"1,2,3,4,5,6,7", ".", 0},
		{"1,2,,2,7,f,s,b,w,,qw", ",", 10},
		{"756", "756789", 0},
		{"", "1", 0},
		{"...", "", 4},
		{"сиплюсплюс", "", 11},
	};
	for (const auto &test : count_tests) {
		STF_ASSERT(strings::count(test.str, test.sep) == test.count);
	}
}

STF_TEST("strings::equal_fold(slice<const char>, slice<const char>)") {
	struct equal_fold_test {
		string a;
		string b;
		bool out;
	};
	vector<equal_fold_test> equal_fold_tests = {
		{"abc", "abc", true},
		{"ABcd", "ABcd", true},
		{"123abc", "123ABC", true},
		{"αβδ", "ΑΒΔ", true},
		{"abc", "xyz", false},
		{"abc", "XYZ", false},
		{"abcdefghijk", "abcdefghijX", false},
		{"abcdefghijk", "abcdefghij\u212A", true},
		{"abcdefghijK", "abcdefghij\u212A", true},
		{"abcdefghijkz", "abcdefghij\u212Ay", false},
		{"abcdefghijKz", "abcdefghij\u212Ay", false},
	};
	for (const auto &test : equal_fold_tests) {
		STF_ASSERT(strings::equal_fold(test.a, test.b) == test.out);
		STF_ASSERT(strings::equal_fold(test.b, test.a) == test.out);
	}
}

STF_TEST("strings::fields(slice<const char>)") {
	struct fields_test {
		string s;
		vector<string> a;
	};
	vector<fields_test> fields_tests = {
		{"", {}},
		{" ", {}},
		{" \t ", {}},
		{"  abc  ", {"abc"}},
		{"1 2 3 4", {"1", "2", "3", "4"}},
		{"1  2  3  4", {"1", "2", "3", "4"}},
		{"1\t\t2\t\t3\t4", {"1", "2", "3", "4"}},
		{"1\u20002\u20013\u20024", {"1", "2", "3", "4"}},
		{"\u2000\u2001\u2002", {}},
		{"\n™\t™\n", {"™", "™"}},
		{"☺☻☹", {"☺☻☹"}},
	};
	for (const auto &test : fields_tests) {
		STF_ASSERT(strings::fields(test.s).sub() == test.a.sub());
		STF_ASSERT(strings::fields_func(test.s, unicode::is_space).sub() == test.a.sub());
	}
}

STF_TEST("strings::fields_func(slice<const char>, func<bool(rune)>)") {
	struct fields_func_test {
		string s;
		vector<string> a;
	};
	vector<fields_func_test> fields_func_tests = {
		{"", {}},
		{"XX", {}},
		{"XXhiXXX", {"hi"}},
		{"aXXbXXXcX", {"a", "b", "c"}},
	};
	for (const auto &test : fields_func_tests) {
		auto a = strings::fields_func(test.s, [](rune c) { return c == 'X'; });
		STF_ASSERT(a.sub() == test.a.sub());
	}
}

STF_TEST("strings::repeat(slice<const char>, int)") {
	struct repeat_test {
		string in;
		string out;
		int count;
	};
	vector<repeat_test> repeat_tests = {
		{"", "", 0},
		{"", "", 1},
		{"", "", 2},
		{"-", "", 0},
		{"-", "-", 1},
		{"-", "----------", 10},
		{"abc ", "abc abc abc ", 3},
	};
	for (const auto &test : repeat_tests) {
		STF_ASSERT(strings::repeat(test.in, test.count) == test.out);
	}
}

STF_TEST("strings::replace(slice<const char>, slice<const char>, slice<const char>, int)") {
	struct replace_test {
		string in;
		string old;
		string _new;
		int n;
		string out;
	};
	vector<replace_test> replace_tests = {
		{"hello", "l", "L", 0, "hello"},
		{"hello", "l", "L", -1, "heLLo"},
		{"hello", "x", "X", -1, "hello"},
		{"", "x", "X", -1, ""},
		{"radar", "r", "<r>", -1, "<r>ada<r>"},
		{"", "", "<>", -1, "<>"},
		{"banana", "a", "<>", -1, "b<>n<>n<>"},
		{"banana", "a", "<>", 1, "b<>nana"},
		{"banana", "a", "<>", 1000, "b<>n<>n<>"},
		{"banana", "an", "<>", -1, "b<><>a"},
		{"banana", "ana", "<>", -1, "b<>na"},
		{"banana", "", "<>", -1, "<>b<>a<>n<>a<>n<>a<>"},
		{"banana", "", "<>", 10, "<>b<>a<>n<>a<>n<>a<>"},
		{"banana", "", "<>", 6, "<>b<>a<>n<>a<>n<>a"},
		{"banana", "", "<>", 5, "<>b<>a<>n<>a<>na"},
		{"banana", "", "<>", 1, "<>banana"},
		{"banana", "a", "a", -1, "banana"},
		{"banana", "a", "a", 1, "banana"},
		{"☺☻☹", "", "<>", -1, "<>☺<>☻<>☹<>"},
	};
	for (const auto &test : replace_tests) {
		STF_ASSERT(strings::replace(test.in, test.old, test._new, test.n) == test.out);
	}
}

STF_TEST("strings::split(slice<const char>, slice<const char>)") {
	// explode tests
	struct explode_test {
		string s;
		int n;
		vector<string> a;
	};
	vector<explode_test> explode_tests = {
		{"", -1, {}},
		{"abcd", 4, {"a", "b", "c", "d"}},
		{"☺☻☹", 3, {"☺", "☻", "☹"}},
		{"abcd", 2, {"a", "bcd"}},
	};
	for (const auto &test : explode_tests) {
		auto a = strings::split_n(test.s, "", test.n);
		STF_ASSERT(a.sub() == test.a.sub());

		auto s = strings::join(a, "");
		STF_ASSERT(test.s == s);
	}

	// split tests
	struct split_test {
		string s;
		string sep;
		int n;
		vector<string> a;
	};
	vector<split_test> split_tests = {
		{"abcd", "a", 0, {}},
		{"abcd", "a", -1, {"", "bcd"}},
		{"abcd", "z", -1, {"abcd"}},
		{"abcd", "", -1, {"a", "b", "c", "d"}},
		{"1,2,3,4", ",", -1, {"1", "2", "3", "4"}},
		{"1....2....3....4", "...", -1, {"1", ".2", ".3", ".4"}},
		{"☺☻☹", "☹", -1, {"☺☻", ""}},
		{"☺☻☹", "~", -1, {"☺☻☹"}},
		{"☺☻☹", "", -1, {"☺", "☻", "☹"}},
		{"1 2 3 4", " ", 3, {"1", "2", "3 4"}},
		{"1 2", " ", 3, {"1", "2"}},
		{"123", "", 2, {"1", "23"}},
		{"123", "", 17, {"1", "2", "3"}},
	};
	for (const auto &test : split_tests) {
		auto a = strings::split_n(test.s, test.sep, test.n);
		STF_ASSERT(a.sub() == test.a.sub());
		if (test.n == 0) {
			continue;
		}
		auto s = strings::join(a, test.sep);
		STF_ASSERT(s == test.s);

		if (test.n < 0) {
			auto b = strings::split(test.s, test.sep);
			STF_ASSERT(a.sub() == b.sub());
		}
	}
}

STF_TEST("strings::split_after(slice<const char>, slice<const char>)") {
	struct split_test {
		string s;
		string sep;
		int n;
		vector<string> a;
	};
	vector<split_test> split_after_tests = {
		{"abcd", "a", -1, {"a", "bcd"}},
		{"abcd", "z", -1, {"abcd"}},
		{"abcd", "", -1, {"a", "b", "c", "d"}},
		{"1,2,3,4", ",", -1, {"1,", "2,", "3,", "4"}},
		{"1....2....3....4", "...", -1, {"1...", ".2...", ".3...", ".4"}},
		{"☺☻☹", "☹", -1, {"☺☻☹", ""}},
		{"☺☻☹", "~", -1, {"☺☻☹"}},
		{"☺☻☹", "", -1, {"☺", "☻", "☹"}},
		{"1 2 3 4", " ", 3, {"1 ", "2 ", "3 4"}},
		{"1 2 3", " ", 3, {"1 ", "2 ", "3"}},
		{"1 2", " ", 3, {"1 ", "2"}},
		{"123", "", 2, {"1", "23"}},
		{"123", "", 17, {"1", "2", "3"}},
	};
	for (const auto &test : split_after_tests) {
		auto a = strings::split_after_n(test.s, test.sep, test.n);
		STF_ASSERT(a.sub() == test.a.sub());

		auto s = strings::join(a, "");
		STF_ASSERT(s == test.s);

		if (test.n < 0) {
			auto b = strings::split_after(test.s, test.sep);
			STF_ASSERT(a.sub() == b.sub());
		}
	}
}

STF_TEST("strings::title(slice<const char>)") {
	struct title_test {
		string in;
		string out;
	};
	vector<title_test> title_tests = {
		{"", ""},
		{"a", "A"},
		{" aaa aaa aaa ", " Aaa Aaa Aaa "},
		{" Aaa Aaa Aaa ", " Aaa Aaa Aaa "},
		{"123a456", "123a456"},
		{"double-blind", "Double-Blind"},
		{"ÿøû", "Ÿøû"},
	};
	for (const auto &test : title_tests) {
		STF_ASSERT(strings::title(test.in) == test.out);
	}
}

STF_TEST("strings::to_lower(slice<const char>)") {
	struct string_test {
		string in;
		string out;
	};
	vector<string_test> lower_tests = {
		{"", ""},
		{"abc", "abc"},
		{"AbC123", "abc123"},
		{"azAZ09_", "azaz09_"},
		{"\u2C6D\u2C6D\u2C6D\u2C6D\u2C6D", "\u0251\u0251\u0251\u0251\u0251"}, // shrinks one byte per char
	};
	for (const auto &test : lower_tests) {
		STF_ASSERT(strings::to_lower(test.in) == test.out);
	}
}

STF_TEST("strings::to_upper(slice<const char>)") {
	struct string_test {
		string in;
		string out;
	};
	vector<string_test> upper_tests = {
		{"", ""},
		{"abc", "ABC"},
		{"AbC123", "ABC123"},
		{"azAZ09_", "AZAZ09_"},
		{"\u0250\u0250\u0250\u0250\u0250", "\u2C6F\u2C6F\u2C6F\u2C6F\u2C6F"}, // grows one byte per char
	};
	for (const auto &test : upper_tests) {
		STF_ASSERT(strings::to_upper(test.in) == test.out);
	}
}

STF_TEST("strings::trim_space(slice<const char>)") {
	struct string_test {
		string in;
		string out;
	};
	string space = "\t\v\r\f\n\u0085\u00a0\u2000\u3000";
	vector<string_test> trim_space_tests = {
		{"", ""},
		{"abc", "abc"},
		{space + "abc" + space, "abc"},
		{" ", ""},
		{" \t\r\n \t\t\r\r\n\n ", ""},
		{" \t\r\n x\t\t\r\r\n\n ", "x"},
		{" \u2000\t\r\n x\t\t\r\r\ny\n \u3000", "x\t\t\r\r\ny"},
		{"1 \t\r\n2", "1 \t\r\n2"},
		{" x\x80", "x\x80"},
		{" x\xc0", "x\xc0"},
		{"x \xc0\xc0 ", "x \xc0\xc0"},
		{"x \xc0", "x \xc0"},
		{"x \xc0 ", "x \xc0"},
		{"x \xc0\xc0 ", "x \xc0\xc0"},
		{"x ☺\xc0\xc0 ", "x ☺\xc0\xc0"},
		{"x ☺ ", "x ☺"},
	};
	for (const auto &test : trim_space_tests) {
		STF_ASSERT(strings::trim_space(test.in) == test.out);
	}
}

STF_TEST("strings::trim(slice<const char>, slice<const char>)") {
	struct trim_test {
		string f;
		string in;
		string arg;
		string out;
	};
	vector<trim_test> trim_tests = {
		{"Trim", "abba", "a", "bb"},
		{"Trim", "abba", "ab", ""},
		{"TrimLeft", "abba", "ab", ""},
		{"TrimRight", "abba", "ab", ""},
		{"TrimLeft", "abba", "a", "bba"},
		{"TrimRight", "abba", "a", "abb"},
		{"Trim", "<tag>", "<>", "tag"},
		{"Trim", "* listitem", " *", "listitem"},
		{"Trim", R"("quote")", R"(")", "quote"},
		{"Trim", "\u2C6F\u2C6F\u0250\u0250\u2C6F\u2C6F", "\u2C6F", "\u0250\u0250"},
		// empty string tests
		{"Trim", "abba", "", "abba"},
		{"Trim", "", "123", ""},
		{"Trim", "", "", ""},
		{"TrimLeft", "abba", "", "abba"},
		{"TrimLeft", "", "123", ""},
		{"TrimLeft", "", "", ""},
		{"TrimRight", "abba", "", "abba"},
		{"TrimRight", "", "123", ""},
		{"TrimRight", "", "", ""},
		{"TrimRight", "☺\xc0", "☺", "☺\xc0"},
		{"TrimPrefix", "aabb", "a", "abb"},
		{"TrimPrefix", "aabb", "b", "aabb"},
		{"TrimSuffix", "aabb", "a", "aabb"},
		{"TrimSuffix", "aabb", "b", "aab"},
	};
	for (const auto &test : trim_tests) {
		auto name = test.f;
		if (name == "Trim") {
			STF_ASSERT(strings::trim(test.in, test.arg) == test.out);
		} else if (name == "TrimLeft") {
			STF_ASSERT(strings::trim_left(test.in, test.arg) == test.out);
		} else if (name == "TrimRight") {
			STF_ASSERT(strings::trim_right(test.in, test.arg) == test.out);
		} else if (name == "TrimPrefix") {
			STF_ASSERT(strings::trim_prefix(test.in, test.arg) == test.out);
		} else if (name == "TrimSuffix") {
			STF_ASSERT(strings::trim_suffix(test.in, test.arg) == test.out);
		} else {
			STF_ASSERT(!"undefined trim function");
		}
	}
}

STF_TEST("strings::trim_func(slice<const char>, func<bool(rune)>)") {
	struct trim_func_test {
		func<bool(rune)> f;
		string in;
		string out;
	};
	auto is_valid_rune = [](rune r) {
		return r != utf8::rune_error;
	};
	auto is_invalid_rune = [](rune r) {
		return r == utf8::rune_error;
	};
	auto is_not_space = [](rune r) {
		return !unicode::is_space(r);
	};
	auto is_not_digit = [](rune r) {
		return !unicode::is_digit(r);
	};
	string space = "\t\v\r\f\n\u0085\u00a0\u2000\u3000";
	vector<trim_func_test> trim_func_tests = {
		{unicode::is_space, space + " hello " + space, "hello"},
		{unicode::is_digit, "\u0e50\u0e5212hello34\u0e50\u0e51", "hello"},
		{unicode::is_upper, "\u2C6F\u2C6F\u2C6F\u2C6FABCDhelloEF\u2C6F\u2C6FGH\u2C6F\u2C6F", "hello"},
		{is_not_space, "hello" + space + "hello", space},
		{is_not_digit, "hello\u0e50\u0e521234\u0e50\u0e51helo", "\u0e50\u0e521234\u0e50\u0e51"},
		{is_valid_rune, "ab\xc0""a\xc0""cd", "\xc0""a\xc0"},
		{is_invalid_rune, "\xc0""a\xc0", "a"},
	};
	for (const auto &test : trim_func_tests) {
		STF_ASSERT(strings::trim_func(test.in, test.f) == test.out);
	}
}
