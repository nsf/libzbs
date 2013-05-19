#include "stf.hh"
#include "zbs/unicode.hh"
#include "zbs/unicode/utf8.hh"
#include "zbs/string.hh"

STF_SUITE_NAME("zbs/unicode");

using namespace zbs;
namespace utf8 = unicode::utf8;

const rune upper_test[] = {
	0x41,
	0xc0,
	0xd8,
	0x100,
	0x139,
	0x14a,
	0x178,
	0x181,
	0x376,
	0x3cf,
	0x1f2a,
	0x2102,
	0x2c00,
	0x2c10,
	0x2c20,
	0xa650,
	0xa722,
	0xff3a,
	0x10400,
	0x1d400,
	0x1d7ca,
};

const rune not_upper_test[] = {
	0x40,
	0x5b,
	0x61,
	0x185,
	0x1b0,
	0x377,
	0x387,
	0x2150,
	0xffff,
	0x10000,
};

const rune letter_test[] = {
	0x41,
	0x61,
	0xaa,
	0xba,
	0xc8,
	0xdb,
	0xf9,
	0x2ec,
	0x535,
	0x620,
	0x6e6,
	0x93d,
	0xa15,
	0xb99,
	0xdc0,
	0xedd,
	0x1000,
	0x1200,
	0x1312,
	0x1401,
	0x1885,
	0x2c00,
	0xa800,
	0xf900,
	0xfa30,
	0xffda,
	0xffdc,
	0x10000,
	0x10300,
	0x10400,
	0x20000,
	0x2f800,
	0x2fa1d,
};

const rune not_letter_test[] = {
	0x20,
	0x35,
	0x375,
	0x619,
	0x700,
	0xfffe,
	0x1ffff,
	0x10ffff,
};

const rune digit_test[] = {
	0x0030,
	0x0039,
	0x0661,
	0x06F1,
	0x07C9,
	0x0966,
	0x09EF,
	0x0A66,
	0x0AEF,
	0x0B66,
	0x0B6F,
	0x0BE6,
	0x0BEF,
	0x0C66,
	0x0CEF,
	0x0D66,
	0x0D6F,
	0x0E50,
	0x0E59,
	0x0ED0,
	0x0ED9,
	0x0F20,
	0x0F29,
	0x1040,
	0x1049,
	0x1090,
	0x1091,
	0x1099,
	0x17E0,
	0x17E9,
	0x1810,
	0x1819,
	0x1946,
	0x194F,
	0x19D0,
	0x19D9,
	0x1B50,
	0x1B59,
	0x1BB0,
	0x1BB9,
	0x1C40,
	0x1C49,
	0x1C50,
	0x1C59,
	0xA620,
	0xA629,
	0xA8D0,
	0xA8D9,
	0xA900,
	0xA909,
	0xAA50,
	0xAA59,
	0xFF10,
	0xFF19,
	0x104A1,
	0x1D7CE,
};

const rune space_test[] = {
	0x09,
	0x0a,
	0x0b,
	0x0c,
	0x0d,
	0x20,
	0x85,
	0xA0,
	0x2000,
	0x3000,
};

STF_TEST("unicode::is_control(rune)") {
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		auto got = unicode::is_control(i);
		auto want = false;
		if (0x00 <= i && i <= 0x1F) {
			want = true;
		} else if (0x7F <= i && i <= 0x9F) {
			want = true;
		}
		STF_ASSERT(got == want);
	}
}

STF_TEST("unicode::is_digit(rune)") {
	for (rune r : digit_test) {
		STF_ASSERT(unicode::is_digit(r));
	}
	for (rune r : letter_test) {
		STF_ASSERT(!unicode::is_digit(r));
	}
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		STF_ASSERT(unicode::is(unicode::digit, i) == unicode::is_digit(i));
	}
}

STF_TEST("unicode::is_graphic(rune)") {
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		// we're testing latin1 table correctness only
		// TODO: more tests?
		STF_ASSERT(unicode::is_graphic(i) == unicode::is_one_of(unicode::graphic_ranges, i));
	}
}

STF_TEST("unicode::is_letter(rune)") {
	for (rune r : upper_test) {
		STF_ASSERT(unicode::is_letter(r));
	}
	for (rune r : letter_test) {
		STF_ASSERT(unicode::is_letter(r));
	}
	for (rune r : not_letter_test) {
		STF_ASSERT(!unicode::is_letter(r));
	}
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		// we're testing latin1 table correctness only
		STF_ASSERT(unicode::is_letter(i) == unicode::is(unicode::letter, i));
	}
}

STF_TEST("unicode::is_lower(rune)") {
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		// we're testing latin1 table correctness only
		// TODO: more tests?
		STF_ASSERT(unicode::is_lower(i) == unicode::is(unicode::lower, i));
	}
}

STF_TEST("unicode::is_number(rune)") {
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		// we're testing latin1 table correctness only
		// TODO: more tests?
		STF_ASSERT(unicode::is_number(i) == unicode::is(unicode::number, i));
	}
}

STF_TEST("unicode::is_print(rune)") {
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		// we're testing latin1 table correctness only
		// TODO: more tests?
		auto want = unicode::is_one_of(unicode::print_ranges, i);
		if (i == ' ') {
			want = true;
		}
		STF_ASSERT(unicode::is_print(i) == want);
	}
}

STF_TEST("unicode::is_punct(rune)") {
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		// we're testing latin1 table correctness only
		// TODO: more tests?
		STF_ASSERT(unicode::is_punct(i) == unicode::is(unicode::punct, i));
	}
}

STF_TEST("unicode::is_space(rune)") {
	for (rune r : space_test) {
		STF_ASSERT(unicode::is_space(r));
	}
	for (rune r : letter_test) {
		STF_ASSERT(!unicode::is_space(r));
	}
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		// we're testing latin1 table correctness only
		STF_ASSERT(unicode::is_space(i) == unicode::is(unicode::White_Space, i));
	}
}

STF_TEST("unicode::is_symbol(rune)") {
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		// we're testing latin1 table correctness only
		// TODO: more tests?
		STF_ASSERT(unicode::is_symbol(i) == unicode::is(unicode::symbol, i));
	}
}

STF_TEST("unicode::is_title(rune)") {
	for (rune i = 0; i <= unicode::max_latin1; i++) {
		// we're testing latin1 table correctness only
		// TODO: more tests?
		STF_ASSERT(unicode::is_title(i) == unicode::is(unicode::title, i));
	}
}

STF_TEST("unicode::is_upper(rune)") {
	for (rune r : upper_test) {
		STF_ASSERT(unicode::is_upper(r));
	}
	for (rune r : not_upper_test) {
		STF_ASSERT(!unicode::is_upper(r));
	}
	for (rune r : not_letter_test) {
		STF_ASSERT(!unicode::is_upper(r));
	}
}

const string simple_fold_tests[] = {
	// SimpleFold could order its returned slices in any order it wants,
	// but we know it orders them in increasing order starting at in
	// and looping around from MaxRune to 0.

	// Easy cases.
	"Aa",
	"aA",
	"δΔ",
	"Δδ",

	// ASCII special cases.
	"KkK",
	"kKK",
	"KKk",
	"Ssſ",
	"sſS",
	"ſSs",

	// Non-ASCII special cases.
	"ρϱΡ",
	"ϱΡρ",
	"Ρρϱ",
	"ͅΙιι",
	"Ιιιͅ",
	"ιιͅΙ",
	"ιͅΙι",

	// Extra special cases: has lower/upper but no case fold.
	"İ",
	"ı",
};

STF_TEST("unicode::simple_fold(rune)") {
	for (const auto &test : simple_fold_tests) {
		auto r = utf8::decode_last_rune(test);
		for (const auto &it : test) {
			auto fr = unicode::simple_fold(r);
			STF_ASSERT(fr == it.rune);
			r = it.rune;
		}
	}
}

struct case_test {
	unicode::case_t cas;
	rune in;
	rune out;
};

const case_test case_tests[] = {
	// errors
	{unicode::case_t(-1), '\n', 0xFFFD},
	{unicode::upper_case, -1, -1},
	{unicode::upper_case, 1 << 30, 1 << 30},

	// ASCII (special-cased so test carefully)
	{unicode::upper_case, '\n', '\n'},
	{unicode::upper_case, 'a', 'A'},
	{unicode::upper_case, 'A', 'A'},
	{unicode::upper_case, '7', '7'},
	{unicode::lower_case, '\n', '\n'},
	{unicode::lower_case, 'a', 'a'},
	{unicode::lower_case, 'A', 'a'},
	{unicode::lower_case, '7', '7'},
	{unicode::title_case, '\n', '\n'},
	{unicode::title_case, 'a', 'A'},
	{unicode::title_case, 'A', 'A'},
	{unicode::title_case, '7', '7'},

	// Latin-1: easy to read the tests!
	{unicode::upper_case, 0x80, 0x80},
	{unicode::upper_case, U'Å', U'Å'},
	{unicode::upper_case, U'å', U'Å'},
	{unicode::lower_case, 0x80, 0x80},
	{unicode::lower_case, U'Å', U'å'},
	{unicode::lower_case, U'å', U'å'},
	{unicode::title_case, 0x80, 0x80},
	{unicode::title_case, U'Å', U'Å'},
	{unicode::title_case, U'å', U'Å'},

	// 0131;LATIN SMALL LETTER DOTLESS I;Ll;0;L;;;;;N;;;0049;;0049
	{unicode::upper_case, 0x0131, 'I'},
	{unicode::lower_case, 0x0131, 0x0131},
	{unicode::title_case, 0x0131, 'I'},

	// 0133;LATIN SMALL LIGATURE IJ;Ll;0;L;<compat> 0069 006A;;;;N;LATIN SMALL LETTER I J;;0132;;0132
	{unicode::upper_case, 0x0133, 0x0132},
	{unicode::lower_case, 0x0133, 0x0133},
	{unicode::title_case, 0x0133, 0x0132},

	// 212A;KELVIN SIGN;Lu;0;L;004B;;;;N;DEGREES KELVIN;;;006B;
	{unicode::upper_case, 0x212A, 0x212A},
	{unicode::lower_case, 0x212A, 'k'},
	{unicode::title_case, 0x212A, 0x212A},

	// From an UpperLower sequence
	// A640;CYRILLIC CAPITAL LETTER ZEMLYA;Lu;0;L;;;;;N;;;;A641;
	{unicode::upper_case, 0xA640, 0xA640},
	{unicode::lower_case, 0xA640, 0xA641},
	{unicode::title_case, 0xA640, 0xA640},
	// A641;CYRILLIC SMALL LETTER ZEMLYA;Ll;0;L;;;;;N;;;A640;;A640
	{unicode::upper_case, 0xA641, 0xA640},
	{unicode::lower_case, 0xA641, 0xA641},
	{unicode::title_case, 0xA641, 0xA640},
	// A64E;CYRILLIC CAPITAL LETTER NEUTRAL YER;Lu;0;L;;;;;N;;;;A64F;
	{unicode::upper_case, 0xA64E, 0xA64E},
	{unicode::lower_case, 0xA64E, 0xA64F},
	{unicode::title_case, 0xA64E, 0xA64E},
	// A65F;CYRILLIC SMALL LETTER YN;Ll;0;L;;;;;N;;;A65E;;A65E
	{unicode::upper_case, 0xA65F, 0xA65E},
	{unicode::lower_case, 0xA65F, 0xA65F},
	{unicode::title_case, 0xA65F, 0xA65E},

	// From another UpperLower sequence
	// 0139;LATIN CAPITAL LETTER L WITH ACUTE;Lu;0;L;004C 0301;;;;N;LATIN CAPITAL LETTER L ACUTE;;;013A;
	{unicode::upper_case, 0x0139, 0x0139},
	{unicode::lower_case, 0x0139, 0x013A},
	{unicode::title_case, 0x0139, 0x0139},
	// 013F;LATIN CAPITAL LETTER L WITH MIDDLE DOT;Lu;0;L;<compat> 004C 00B7;;;;N;;;;0140;
	{unicode::upper_case, 0x013f, 0x013f},
	{unicode::lower_case, 0x013f, 0x0140},
	{unicode::title_case, 0x013f, 0x013f},
	// 0148;LATIN SMALL LETTER N WITH CARON;Ll;0;L;006E 030C;;;;N;LATIN SMALL LETTER N HACEK;;0147;;0147
	{unicode::upper_case, 0x0148, 0x0147},
	{unicode::lower_case, 0x0148, 0x0148},
	{unicode::title_case, 0x0148, 0x0147},

	// Last block in the 5.1.0 table
	// 10400;DESERET CAPITAL LETTER LONG I;Lu;0;L;;;;;N;;;;10428;
	{unicode::upper_case, 0x10400, 0x10400},
	{unicode::lower_case, 0x10400, 0x10428},
	{unicode::title_case, 0x10400, 0x10400},
	// 10427;DESERET CAPITAL LETTER EW;Lu;0;L;;;;;N;;;;1044F;
	{unicode::upper_case, 0x10427, 0x10427},
	{unicode::lower_case, 0x10427, 0x1044F},
	{unicode::title_case, 0x10427, 0x10427},
	// 10428;DESERET SMALL LETTER LONG I;Ll;0;L;;;;;N;;;10400;;10400
	{unicode::upper_case, 0x10428, 0x10400},
	{unicode::lower_case, 0x10428, 0x10428},
	{unicode::title_case, 0x10428, 0x10400},
	// 1044F;DESERET SMALL LETTER EW;Ll;0;L;;;;;N;;;10427;;10427
	{unicode::upper_case, 0x1044F, 0x10427},
	{unicode::lower_case, 0x1044F, 0x1044F},
	{unicode::title_case, 0x1044F, 0x10427},

	// First one not in the 5.1.0 table
	// 10450;SHAVIAN LETTER PEEP;Lo;0;L;;;;;N;;;;;
	{unicode::upper_case, 0x10450, 0x10450},
	{unicode::lower_case, 0x10450, 0x10450},
	{unicode::title_case, 0x10450, 0x10450},

	// Non-letters with case.
	{unicode::lower_case, 0x2161, 0x2171},
	{unicode::upper_case, 0x0345, 0x0399},
};

STF_TEST("unicode::to(case_t, rune)") {
	for (const auto &test : case_tests) {
		STF_ASSERT(unicode::to(test.cas, test.in) == test.out);
	}
}

STF_TEST("unicode::to_upper(rune)") {
	for (const auto &test : case_tests) {
		if (test.cas != unicode::upper_case)
			continue;
		STF_ASSERT(unicode::to_upper(test.in) == test.out);
	}
}

STF_TEST("unicode::to_lower(rune)") {
	for (const auto &test : case_tests) {
		if (test.cas != unicode::lower_case)
			continue;
		STF_ASSERT(unicode::to_lower(test.in) == test.out);
	}
}

STF_TEST("unicode::to_title(rune)") {
	for (const auto &test : case_tests) {
		if (test.cas != unicode::title_case)
			continue;
		STF_ASSERT(unicode::to_title(test.in) == test.out);
	}
}

// TODO: port more tests from Go
