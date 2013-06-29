// This file may contain portions of the code derived/adopted from the Go
// standard library, which is covered by a BSD-style license. You can find more
// details in 3rdparty/go_license.txt file.

#include "stf.hh"
#include "zbs.hh"
#include "zbs/slices.hh"

STF_SUITE_NAME("zbs::slices");

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

STF_TEST("slices::count(slice<T>, slice<U>)") {
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
	};
	for (const auto &test : count_tests) {
		STF_ASSERT(slices::count(test.str.sub(), test.sep.sub()) == test.count);
	}
}

STF_TEST("slices::starts_with(slice<T>, slice<U>)") {
	struct test {
		string str;
		string prefix;
		bool result;
	};
	vector<test> tests = {
		{"12345", "123", true},
		{"12345", "132", false},
		{"17", "", true},
		{"", "", true},
		{"14", "1456", false},
	};
	for (const auto &test : tests) {
		STF_ASSERT(slices::starts_with(test.str.sub(), test.prefix.sub()) == test.result);
	}
}

STF_TEST("slices::ends_with(slice<T>, slice<U>)") {
	struct test {
		string str;
		string suffix;
		bool result;
	};
	vector<test> tests = {
		{"12345", "45", true},
		{"12345", "54", false},
		{"17", "", true},
		{"", "", true},
		{"14", "5614", false},
		{"14", "1456", false},
	};
	for (const auto &test : tests) {
		STF_ASSERT(slices::ends_with(test.str.sub(), test.suffix.sub()) == test.result);
	}
}

STF_TEST("slices::index_any(slice<T>, slice<U>)") {
	struct test {
		string str;
		string elems;
		int where;
	};
	vector<test> tests = {
		{"", "", -1},
		{"", "a", -1},
		{"", "abc", -1},
		{"a", "", -1},
		{"a", "a", 0},
		{"aaa", "a", 0},
		{"abc", "xyz", -1},
		{"abc", "xcz", 2},
		{"aRegExp*", ".(|)*+?^$[]", 7},
	};
	for (const auto &test : tests) {
		STF_ASSERT(slices::index_any(test.str.sub(), test.elems.sub()) == test.where);
	}
}

STF_TEST("slices::last_index(slice<T>, slice<U>)") {
	struct test {
		string str;
		string substr;
		int where;
	};
	vector<test> tests = {
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
	for (const auto &test : tests) {
		STF_ASSERT(slices::last_index(test.str.sub(), test.substr.sub()) == test.where);
	}
}

STF_TEST("slices::last_index_any(slice<T>, slice<U>)") {
	struct test {
		string str;
		string elems;
		int where;
	};
	vector<test> tests = {
		{"", "", -1},
		{"", "a", -1},
		{"", "abc", -1},
		{"a", "", -1},
		{"a", "a", 0},
		{"aaa", "a", 2},
		{"abc", "xyz", -1},
		{"abc", "ab", 1},
		{"a.RegExp*", ".(|)*+?^$[]", 8},
	};
	for (const auto &test : tests) {
		STF_ASSERT(slices::last_index_any(test.str.sub(), test.elems.sub()) == test.where);
	}
}

STF_TEST("slices::sort(slice<T>)") {
	vector<string> strings = {
		"bbbb",
		"aaa",
		"dddd",
		"ccc",
		"CCCC",
		"AAA",
		"GGGG",
		"HHH",
	};
	slices::sort(strings.sub());
	const string *prev = nullptr;
	for (const auto &cur : strings) {
		if (prev) {
			STF_ASSERT(*prev < cur);
		}
		prev = &cur;
	}
}
