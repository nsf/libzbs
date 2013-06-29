#pragma once

#include "_types.hh"
#include "_slice.hh"
#include "_map.hh"

namespace zbs {
namespace unicode {

constexpr rune max_rune = U'\U0010FFFF';
constexpr rune replacement_char = U'\uFFFD';
constexpr rune max_ascii = U'\u007F';
constexpr rune max_latin1 = U'\u00FF';

struct range16 {
	uint16 lo;
	uint16 hi;
	uint16 stride;
};

struct range32 {
	uint32 lo;
	uint32 hi;
	uint32 stride;
};

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

bool is(const range_table &rt, rune r);
bool is_control(rune r);
bool is_digit(rune r);
bool is_graphic(rune r);
bool is_letter(rune r);
bool is_lower(rune r);
bool is_mark(rune r);
bool is_number(rune r);
bool is_one_of(slice<const range_table> set, rune r);
bool is_print(rune r);
bool is_punct(rune r);
bool is_space(rune r);
bool is_symbol(rune r);
bool is_title(rune r);
bool is_upper(rune r);
rune simple_fold(rune r);
rune to(case_t _case, rune r);
rune to_lower(rune r);
rune to_title(rune r);
rune to_upper(rune r);

}} // namespace zbs::unicode

#include "_unicode_tables.hh"
