// This file may contain portions of the code derived/adopted from the Go
// standard library, which is covered by a BSD-style license. You can find more
// details in 3rdparty/go_license.txt file.

#include "zbs/unicode.hh"
#include "zbs/_slice.hh"

namespace zbs {
namespace unicode {

enum {
	pC = 1 << 0,     // a control character
	pP = 1 << 1,     // a punctuation character
	pN = 1 << 2,     // a numeral
	pS = 1 << 3,     // a symbolic character
	pZ = 1 << 4,     // a spacing character
	pLu = 1 << 5,    // an upper-case letter
	pLl = 1 << 6,    // a lower-case letter
	pp = 1 << 7,     // a printable character according to Go's definition
	pg = pp | pZ,    // a graphical character according to the Unicode definition
	pLo = pLl | pLu, // a letter that is neither upper nor lower case
	pLmask = pLo,
};

struct fold_pair {
	uint16 from;
	uint16 to;
};

#include "unicode_private_tables.inl"

static const int linear_max = 18;

// T is range16 or range32
template <typename T, typename U>
static bool is_in_ranges(slice<T> ranges, U r) {
	if (ranges.len() <= linear_max || r <= (U)max_latin1) {
		for (const auto &range: ranges) {
			if (r < range.lo) {
				return false;
			}
			if (r <= range.hi) {
				return (r-range.lo) % range.stride == 0;
			}
		}
		return false;
	}

	// binary search over ranges
	int lo = 0;
	int hi = ranges.len();
	while (lo < hi) {
		int m = lo + (hi-lo) / 2;
		const auto &range = ranges[m];
		if (range.lo <= r && r <= range.hi) {
			return (r-range.lo) % range.stride == 0;
		}
		if (r < range.lo) {
			hi = m;
		} else {
			lo = m + 1;
		}
	}
	return false;
}

bool is(const range_table &rt, rune r) {
	auto r16 = rt.r16;
	if (r16.len() > 0 && r <= rune(r16[r16.len()-1].hi)) {
		return is_in_ranges(r16, uint16(r));
	}
	auto r32 = rt.r32;
	if (r32.len() > 0 && r >= rune(r32[0].lo)) {
		return is_in_ranges(r32, uint32(r));
	}
	return false;
}

static bool is_excluding_latin(const range_table &rt, rune r) {
	auto r16 = rt.r16;
	int off = rt.latin_offset;
	if (r16.len() > off && r <= rune(r16[r16.len()-1].hi)) {
		return is_in_ranges(r16, uint16(r));
	}
	auto r32 = rt.r32;
	if (r32.len() > 0 && r >= rune(r32[0].lo)) {
		return is_in_ranges(r32, uint32(r));
	}
	return false;
}

bool is_control(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return latin_properties[uint8(r)] & pC;
	}
	return false;
}

bool is_digit(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return '0' <= r && r <= '9';
	}
	return is_excluding_latin(digit, r);
}

bool is_graphic(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return latin_properties[uint8(r)] & pg;
	}
	return is_one_of(graphic_ranges, r);
}

bool is_letter(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return latin_properties[uint8(r)] & pLmask;
	}
	return is_excluding_latin(letter, r);
}

bool is_lower(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return (latin_properties[uint8(r)] & pLmask) == pLl;
	}
	return is_excluding_latin(lower, r);
}

bool is_mark(rune r) {
	// There are no mark characters in Latin-1.
	return is_excluding_latin(mark, r);
}

bool is_number(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return latin_properties[uint8(r)] & pN;
	}
	return is_excluding_latin(number, r);
}

bool is_one_of(slice<const range_table> set, rune r) {
	for (const auto &inside : set) {
		if (is(inside, r)) {
			return true;
		}
	}
	return false;
}

bool is_print(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return latin_properties[uint8(r)] & pp;
	}
	return is_one_of(print_ranges, r);
}

bool is_punct(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return latin_properties[uint8(r)] & pP;
	}
	return is_excluding_latin(punct, r);
}

bool is_space(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return r == '\t' || r == '\n' ||
			r == '\v' || r == '\f' ||
			r == '\r' || r == ' ' ||
			r == 0x85 || r == 0xA0;
	}
	return is_excluding_latin(White_Space, r);
}

bool is_symbol(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return latin_properties[uint8(r)] & pS;
	}
	return is_excluding_latin(symbol, r);
}

bool is_title(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return false;
	}
	return is_excluding_latin(title, r);
}

bool is_upper(rune r) {
	if (uint32(r) <= uint32(max_latin1)) {
		return (latin_properties[uint8(r)] & pLmask) == pLu;
	}
	return is_excluding_latin(upper, r);
}

rune simple_fold(rune r) {
	// Consult caseOrbit table for special cases.
	int lo = 0;
	int hi = case_orbit.len();
	while (lo < hi) {
		int m = lo + (hi-lo) / 2;
		if (rune(case_orbit[m].from) < r) {
			lo = m + 1;
		} else {
			hi = m;
		}
	}
	if (lo < case_orbit.len() && case_orbit[lo].from == r) {
		return case_orbit[lo].to;
	}

	// No folding specified. This is a one- or two-element
	// equivalence class containing rune and to_lower(rune)
	// and to_upper(rune) if they are different from rune.
	int l = to_lower(r);
	return (l != r) ? l : to_upper(r);
}

rune to_cr(case_t _case, rune r, slice<const case_range> crs) {
	if (_case < 0 || max_case <= _case) {
		return replacement_char;
	}

	int lo = 0;
	int hi = crs.len();
	while (lo < hi) {
		int m = lo + (hi-lo)/2;
		const auto &cr = crs[m];
		if (rune(cr.lo) <= r && r <= rune(cr.hi)) {
			auto delta = cr.delta[_case];
			if (delta > max_rune) {
				// In an upper-lower sequence, which always
				// starts with an upper_case letter, the real
				// deltas always look like:
				//      {0, 1, 0}    upper_case (lower is next)
				//      {-1, 0, -1}  lower_case (upper, title are previous)
				// The characters at even offsets from the
				// beginning of the sequence are upper case;
				// the ones at odd offsets are lower. The
				// correct mapping can be done by clearing or
				// setting the low bit in the sequence offset.
				// The constants upper_case and title_case are
				// even while lower_case is odd so we take the
				// low bit from _case.
				return rune(cr.lo) + (((r - rune(cr.lo)) & ~1) | rune(_case & 1));
			}
			return r + delta;
		}
		if (r < rune(cr.lo)) {
			hi = m;
		} else {
			lo = m + 1;
		}
	}
	return r;
}

rune to(case_t _case, rune r) {
	return to_cr(_case, r, case_ranges);
}

rune to_lower(rune r) {
	if (uint32(r) <= max_ascii) {
		if ('A' <= r && r <= 'Z') {
			r += 'a' - 'A';
		}
		return r;
	}
	return to(lower_case, r);
}

rune to_title(rune r) {
	if (uint32(r) <= max_ascii) {
		if ('a' <= r && r <= 'z') {
			r -= 'a' - 'A';
		}
		return r;
	}
	return to(title_case, r);
}

rune to_upper(rune r) {
	if (uint32(r) <= max_ascii) {
		if ('a' <= r && r <= 'z') {
			r -= 'a' - 'A';
		}
		return r;
	}
	return to(upper_case, r);
}

}} // namespace zbs::unicode
