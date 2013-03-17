// This file may contain portions of the code derived/adopted from the Go
// standard library, which is covered by a BSD-style license. You can find more
// details in 3rdparty/go_license.txt file.

#include "stf.hh"
#include "zbs/vector.hh"
#include "zbs/string.hh"
#include "zbs/unicode/utf8.hh"

STF_SUITE_NAME("zbs/unicode/utf8");

using namespace zbs;
namespace utf8 = unicode::utf8;

struct utf8map_t {
	rune r;
	string str;
};

utf8map_t utf8map[] = {
        {0x0000, slice<const char>{"\0", 1}},
        {0x0001, "\x01"},
        {0x007e, "\x7e"},
        {0x007f, "\x7f"},
        {0x0080, "\xc2\x80"},
        {0x0081, "\xc2\x81"},
        {0x00bf, "\xc2\xbf"},
        {0x00c0, "\xc3\x80"},
        {0x00c1, "\xc3\x81"},
        {0x00c8, "\xc3\x88"},
        {0x00d0, "\xc3\x90"},
        {0x00e0, "\xc3\xa0"},
        {0x00f0, "\xc3\xb0"},
        {0x00f8, "\xc3\xb8"},
        {0x00ff, "\xc3\xbf"},
        {0x0100, "\xc4\x80"},
        {0x07ff, "\xdf\xbf"},
        {0x0800, "\xe0\xa0\x80"},
        {0x0801, "\xe0\xa0\x81"},
        {0xd7ff, "\xed\x9f\xbf"}, // last code point before surrogate half.
        {0xe000, "\xee\x80\x80"}, // first code point after surrogate half.
        {0xfffe, "\xef\xbf\xbe"},
        {0xffff, "\xef\xbf\xbf"},
        {0x10000, "\xf0\x90\x80\x80"},
        {0x10001, "\xf0\x90\x80\x81"},
        {0x10fffe, "\xf4\x8f\xbf\xbe"},
        {0x10ffff, "\xf4\x8f\xbf\xbf"},
        {0xFFFD, "\xef\xbf\xbd"},
};

utf8map_t surrogate_map[] = {
	{0xd800, "\xed\xa0\x80"}, // surrogate min decodes to (RuneError, 1)
	{0xdfff, "\xed\xbf\xbf"}, // surrogate max decodes to (RuneError, 1)
};

string test_strings[] = {
	"",
	"abcd",
	"☺☻☹",
	"日a本b語ç日ð本Ê語þ日¥本¼語i日©",
	"日a本b語ç日ð本Ê語þ日¥本¼語i日©日a本b語ç日ð本Ê語þ日¥本¼語i日©日a本b語ç日ð本Ê語þ日¥本¼語i日©",
	"\x80\x80\x80\x80",
};

struct rune_count_test {
	string in;
	int out;
};

rune_count_test rune_count_tests[] = {
	{"abcd", 4},
	{"☺☻☹", 3},
	{"1,2,3,4", 7},
	{slice<const char>("\xe2\x00", 2), 2},
};

struct rune_len_test {
	rune r;
	int size;
};

rune_len_test rune_len_tests[] = {
	{0, 1},
	{U'e', 1},
	{U'é', 2},
	{U'☺', 3},
	{utf8::rune_error, 3},
	{utf8::max_rune, 4},
	{0xD800, -1},
	{0xDFFF, -1},
	{utf8::max_rune + 1, -1},
	{-1, -1},
};

struct valid_test {
	string in;
	bool out;
};

valid_test valid_tests[] = {
	{"", true},
	{"a", true},
	{"abc", true},
	{"Ж", true},
	{"ЖЖ", true},
	{"брэд-ЛГТМ", true},
	{"☺☻☹", true},
	{string({66, char(250)}), false},
	{string({66, char(250), 67}), false},
	{"a\uFFFDb", true},
	{"\xF4\x8F\xBF\xBF", true},      // U+10FFFF
	{"\xF4\x90\x80\x80", false},     // U+10FFFF+1; out of range
	{"\xF7\xBF\xBF\xBF", false},     // 0x1FFFFF; out of range
	{"\xFB\xBF\xBF\xBF\xBF", false}, // 0x3FFFFFF; out of range
	{"\xc0\x80", false},             // U+0000 encoded in two bytes: incorrect
	{"\xed\xa0\x80", false},         // U+D800 high surrogate (sic)
	{"\xed\xbf\xbf", false},         // U+DFFF low surrogate (sic)
};

struct valid_rune_test {
	rune in;
	bool out;
};

valid_rune_test valid_rune_tests[] = {
	{0, true},
	{'e', true},
	{U'é', true},
	{U'☺', true},
	{utf8::rune_error, true},
	{utf8::max_rune, true},
	{0xD7FF, true},
	{0xD800, false},
	{0xDFFF, false},
	{0xE000, true},
	{utf8::max_rune + 1, false},
	{-1, false},
};

STF_TEST("utf8::full_rune(slice<const char>)") {
	for (const auto &m : utf8map) {
		STF_ASSERT(utf8::full_rune(m.str));
		auto s = m.str.sub(0, m.str.len()-1);
		STF_ASSERT(!utf8::full_rune(s));
	}
}

STF_TEST("utf8::encode_rune(slice<char>, rune)") {
	for (const auto &m : utf8map) {
		char buf[10];
		int n = utf8::encode_rune(buf, m.r);
		STF_ASSERT(slice<const char>(buf, n) == m.str.sub());
	}

	// check that negative runes encode as rune_error
	vector<char> errbuf(utf8::utf_max);
	errbuf.resize(utf8::encode_rune(errbuf, utf8::rune_error));
	vector<char> buf(utf8::utf_max);
	buf.resize(utf8::encode_rune(buf, -1));
	STF_ASSERT(errbuf == buf);
}

STF_TEST("utf8::decode_rune(slice<const char>, int*)") {
	for (const auto &m : utf8map) {
		int size;
		string str = m.str;
		rune r = utf8::decode_rune(str, &size);
		STF_ASSERT(r == m.r && size == str.len());

		// make sure trailing byte works
		str.append({"\0", 1});
		r = utf8::decode_rune(str, &size);
		STF_ASSERT(r == m.r && size == str.len()-1);

		// remove trailing \0
		str.remove(str.len()-1);

		// make sure missing bytes fail
		int wantsize = 1;
		if (wantsize >= str.len()) {
			wantsize = 0;
		}

		r = utf8::decode_rune(str.sub(0, str.len()-1), &size);
		STF_ASSERT(r == utf8::rune_error && size == wantsize);

		// make sure bad sequences fail
		if (str.len() == 1) {
			str[0] = 0x80;
		} else {
			str[str.len()-1] = 0x7F;
		}
		r = utf8::decode_rune(str, &size);
		STF_ASSERT(r == utf8::rune_error && size == 1);
	}

	// surrogate runes
	for (const auto &m : surrogate_map) {
		int size;
		rune r = utf8::decode_rune(m.str, &size);
		STF_ASSERT(r == utf8::rune_error && size == 1);
	}
}

bool test_sequence(slice<const char> s) {
	struct info {
		int index;
		rune r;
	};
	vector<info> index(s.len());
	int j = 0, i = 0;
	while (i < s.len()) {
		int size;
		rune r = utf8::decode_rune(s.sub(i), &size);
		index[j++] = {i, r};
		i += size;
	}
	j--;
	i = s.len();
	while (i > 0) {
		int size;
		rune r = utf8::decode_last_rune(s.sub(0, i), &size);
		if (index[j].r != r) {
			return false;
		}
		i -= size;
		if (index[j].index != i) {
			return false;
		}
		j--;
	}
	return true;
}

STF_TEST("utf8::decode_last_rune(slice<const char>, int*)") {
	// We actually test here that `decode_last_rune` corresponds to
	// `decode_rune`, because `decode_rune` works according to test above.
	for (const auto &ts : test_strings) {
		for (const auto &m : utf8map) {
			for (const auto &s : {ts + m.str, m.str + ts, ts + m.str + ts}) {
				STF_ASSERT(test_sequence(s));
			}
		}
	}
}

STF_TEST("utf8::rune_count(slice<const char>)") {
	for (const auto &t : rune_count_tests) {
		STF_ASSERT(utf8::rune_count(t.in) == t.out);
	}
}

STF_TEST("utf8::rune_len(rune)") {
	for (const auto &t : rune_len_tests) {
		STF_ASSERT(utf8::rune_len(t.r) == t.size);
	}
}

STF_TEST("utf8::valid(slice<const char>)") {
	for (const auto &t : valid_tests) {
		STF_ASSERT(utf8::valid(t.in) == t.out);
	}
}

STF_TEST("utf8::valid_rune(rune)") {
	for (const auto &t : valid_rune_tests) {
		STF_ASSERT(utf8::valid_rune(t.in) == t.out);
	}
}

STF_TEST("utf8::rune_start(char)") {
	for (const auto &m : utf8map) {
		char first = m.str[0];
		STF_ASSERT(utf8::rune_start(first));
		if (m.str.len() > 1) {
			char second = m.str[1];
			STF_ASSERT(!utf8::rune_start(second));
		}
	}
}
