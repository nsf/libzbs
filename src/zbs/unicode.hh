#pragma once

#include "_types.hh"
#include "_slice.hh"
#include "_map.hh"

namespace zbs {
namespace unicode {

/// Maximum valid Unicode code point.
constexpr rune max_rune = U'\U0010FFFF';

/// Represents invalid code points.
constexpr rune replacement_char = U'\uFFFD';

/// Maximum ASCII value.
constexpr rune max_ascii = U'\u007F';

/// Maximum Latin-1 value.
constexpr rune max_latin1 = U'\u00FF';

/// Represents a range of 16-bit Unicode code points. The range runs from lo to
/// hi inclusive and has the specified stride.
struct range16 {
	uint16 lo;
	uint16 hi;
	uint16 stride;
};

/// Represents a range of Unicode code points and is used when one or more of
/// the values will not fit in 16 bits. The range runs from lo to hi inclusive
/// and has the specified stride. Lo and hi must always be >= 1<<16.
struct range32 {
	uint32 lo;
	uint32 hi;
	uint32 stride;
};

/// Class defines a set of Unicode code points by listing the ranges of code
/// points within the set. The ranges are listed in two slices to save space: a
/// slice of 16-bit ranges and a slice of 32-bit ranges. The two slices must be
/// in sorted order and non-overlapping. Also, r32 should contain only values
/// >= 0x10000 (1<<16).
struct range_table {
	slice<const range16> r16;
	slice<const range32> r32;
	int latin_offset = 0;

	constexpr range_table() = default;

	// the reason for this constructor to be explicitly defined is the
	// following: C++ seems to lose array size information in some cases,
	// the one I care about is this: `range_table r = {array, array, n}`
	// doesn't work unfortunately without the constructor below
	constexpr range_table(slice<const range16> r16, slice<const range32> r32, int latin_offset):
		r16(r16), r32(r32), latin_offset(latin_offset) {}
};

enum case_t {
	upper_case,
	lower_case,
	title_case,
	max_case,
};

struct case_range {
	uint32 lo;
	uint32 hi;
	rune delta[max_case];
};

constexpr rune upper_lower = max_rune + 1;

/// Reports whether r is in the specified table of ranges rt.
bool is(const range_table &rt, rune r);

/// Reports whether r is a control character. The C (Other) Unicode category
/// includes more code points such as surrogates; use is(C, r) to test for them.
bool is_control(rune r);

/// Reports whether r is a decimal digit.
bool is_digit(rune r);

/// Reports whether r is defined as a Graphic by Unicode. Such characters
/// include letters, marks, numbers, punctuation, symbols, and spaces, from
/// categories L, M, N, P, S, Zs.
bool is_graphic(rune r);

/// Reports whether r is a letter (category L).
bool is_letter(rune r);

/// Reports whether r is a lower case letter.
bool is_lower(rune r);

/// Reports whether r is a mark character (category M).
bool is_mark(rune r);

/// Reports whether r is a number (category N).
bool is_number(rune r);

/// Reports whether r is a member of one of the ranges in set.
bool is_one_of(slice<const range_table> set, rune r);

/// Reports whether r is defined as printable by libzbs. Such characters
/// include letters, marks, numbers, punctuation, symbols, and the ASCII space
/// character, from categories L, M, N, P, S and the ASCII space character.
/// This categorization is the same as is_graphic except that the only spacing
/// character is ASCII space, U+0020.
bool is_print(rune r);

/// Reports whether r is a Unicode punctuation character (category P).
bool is_punct(rune r);

/// Reports whether r is a space character as defined by Unicode's White Space
/// property; in the Latin-1 space this is: `\t`, `\n`, `\v`, `\f`, `\r`,
/// `U+0020` (SPACE), `U+0085` (NEL), `U+00A0` (NBSP).
bool is_space(rune r);

/// Reports whether r is a symbolic character.
bool is_symbol(rune r);

/// Reports whether r is a title case letter.
bool is_title(rune r);

/// Reports whether r is an upper case letter.
bool is_upper(rune r);

/// Iterates over Unicode code points equivalent under the Unicode-defined
/// simple case folding. Among the code points equivalent to r (including r
/// itself), simple_fold returns the smallest rune >= r if one exists, or else
/// the smallest rune >= 0.
///
/// For example:
///
///     simple_fold(U'A') = U'a'
///     simple_fold(U'a') = U'A'
///
///     simple_fold(U'K') = U'k'
///     simple_fold(U'k') = U'\u212A' (Kelvin symbol, K)
///     simple_fold(U'\u212A') = U'K'
///
///     simple_fold(U'1') = U'1'
rune simple_fold(rune r);

/// Maps r to the specified _case: upper_case, lower_case, or title_case.
rune to(case_t _case, rune r);

/// Maps r to lower case.
rune to_lower(rune r);

/// Maps r to title case.
rune to_title(rune r);

/// Maps r to upper case.
rune to_upper(rune r);

}} // namespace zbs::unicode

#include "_unicode_tables.hh"
