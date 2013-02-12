#include "stf.hh"
#include "zbs/slices.hh"
#include "zbs/string.hh"
#include "zbs/vector.hh"

STF_SUITE_NAME("zbs/slices");

using namespace zbs;

STF_TEST("slices::reverse(slice<T>)") {
	string a = "hello";
	slices::reverse(a.sub());
	STF_ASSERT(a == "olleh");

	string b = "b";
	slices::reverse(b.sub());
	STF_ASSERT(b == "b");

	string c = "C3";
	slices::reverse(c.sub());
	STF_ASSERT(c == "3C");

	string d = "C3P";
	slices::reverse(d.sub());
	STF_ASSERT(d == "P3C");

	string e = "";
	slices::reverse(e.sub());
	STF_ASSERT(e == "");
}

STF_TEST("slices::index(slice<T>, slice<U>)") {
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
		STF_ASSERT(slices::index(test.s.sub(), test.sep.sub()) == test.out);
	}
}

STF_TEST("slices::contains(slice<T>, slice<U>)") {
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
		STF_ASSERT(slices::contains(test.str.sub(), test.substr.sub()) == test.expected);
	}
}
