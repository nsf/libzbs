// This file may contain portions of the code derived/adopted from the Go
// standard library, which is covered by a BSD-style license. You can find more
// details in 3rdparty/go_license.txt file.

#include "zbs/unicode/utf8.hh"

enum {
	t1 = 0x00, // 0000 0000
	tx = 0x80, // 1000 0000
	t2 = 0xC0, // 1100 0000
	t3 = 0xE0, // 1110 0000
	t4 = 0xF0, // 1111 0000
	t5 = 0xF8, // 1111 1000

	maskx = 0x3F, // 0011 1111
	mask2 = 0x1F, // 0001 1111
	mask3 = 0x0F, // 0000 1111
	mask4 = 0x07, // 0000 0111

	rune1max = (1 << 7) - 1,
	rune2max = (1 << 11) - 1,
	rune3max = (1 << 16) - 1,

	// code points in the surrogate range are not valid for UTF-8
	surrogate_min = 0xD800,
	surrogate_max = 0xDFFF,
};

#define OPTRET(x) if (x) *x

namespace zbs {
namespace unicode {
namespace utf8 {

struct decoded_rune {
	rune r;
	int size;
	bool incomplete;
};

static decoded_rune decode_rune_internal(slice<const char> s) {
	int n = s.len();
	if (n < 1) {
		return {rune_error, 0, true};
	}
	byte c0 = s[0];

	// 1-byte, 7-bit sequence?
	if (c0 < tx) {
		return {c0, 1, false};
	}

	// unexpected continuation byte?
	if (c0 < t2) {
		return {rune_error, 1, false};
	}

	// need first continuation byte
	if (n < 2) {
		return {rune_error, 1, true};
	}

	byte c1 = s[1];
	if (c1 < tx || t2 <= c1) {
		return {rune_error, 1, false};
	}

	// 2-byte, 11-bit sequence?
	if (c0 < t3) {
		rune r = rune(c0 & mask2) << 6 | rune(c1 & maskx);
		if (r <= rune1max) {
			return {rune_error, 1, false};
		}
		return {r, 2, false};
	}

	// need second continuation byte
	if (n < 3) {
		return {rune_error, 1, true};
	}

	byte c2 = s[2];
	if (c2 < tx || t2 <= c2) {
		return {rune_error, 1, false};
	}

	// 3-byte, 16-bit sequence?
	if (c0 < t4) {
		rune r = rune(c0 & mask3) << 12 |
			rune(c1 & maskx) << 6 |
			rune(c2 & maskx);
		if (r <= rune2max) {
			return {rune_error, 1, false};
		}
		if (surrogate_min <= r && r <= surrogate_max) {
			return {rune_error, 1, false};
		}
		return {r, 3, false};
	}

	// need third continuation byte
	if (n < 4) {
		return {rune_error, 1, true};
	}

	byte c3 = s[3];
	if (c3 < tx || t2 <= c3) {
		return {rune_error, 1, false};
	}

	// 4-byte, 21-bit sequence?
	if (c0 < t5) {
		rune r = rune(c0 & mask4) << 18 |
			rune(c1 & maskx) << 12 |
			rune(c2 & maskx) << 6 |
			rune(c3 & maskx);
		if (r <= rune3max || max_rune < r) {
			return {rune_error, 1, false};
		}
		return {r, 4, false};
	}

	// error
	return {rune_error, 1, false};
}

bool full_rune(slice<const char> s) {
	return !decode_rune_internal(s).incomplete;
}

sized_rune decode_rune(slice<const char> s) {
	auto r = decode_rune_internal(s);
	return {r.r, r.size};
}

sized_rune decode_last_rune(slice<const char> s) {
	int end = s.len();
	if (end == 0) {
		return {rune_error, 0};
	}
	int start = end - 1;
	rune r = uint8(s[start]);
	if (r < rune_self) {
		return {r, 1};
	}

	// guard against O(n^2) behavior when traversing
	// backwards through strings with long sequences of
	// invalid UTF-8.
	int lim = end - utf_max;
	if (lim < 0) {
		lim = 0;
	}

	for (start--; start >= lim; start--) {
		if (rune_start(s[start])) {
			break;
		}
	}

	if (start < 0) {
		start = 0;
	}

	auto dr = decode_rune_internal(s.sub(start));
	if (start + dr.size != end) {
		return {rune_error, 1};
	}
	return {dr.r, dr.size};
}

int rune_len(rune r) {
	if (r < 0) {
		return -1;
	} else if (r <= rune1max) {
		return 1;
	} else if (r <= rune2max) {
		return 2;
	} else if (surrogate_min <= r && r <= surrogate_max) {
		return -1;
	} else if (r <= rune3max) {
		return 3;
	} else if (r <= max_rune) {
		return 4;
	}
	return -1;
}

int encode_rune(slice<char> s, rune r) {
	uint32 ur = r;
	if (ur <= rune1max) {
		s[0] = char(r);
		return 1;
	}

	if (ur <= rune2max) {
		s[0] = t2 | char(r >> 6);
		s[1] = tx | (char(r) & maskx);
		return 2;
	}

	if (ur > uint32(max_rune)) {
		r = rune_error;
	}

	if (surrogate_min <= r && r <= surrogate_max) {
		r = rune_error;
	}
	ur = r;

	if (ur <= rune3max) {
		s[0] = t3 | char(r >> 12);
		s[1] = tx | (char(r >> 6) & maskx);
		s[2] = tx | (char(r) & maskx);
		return 3;
	}

	s[0] = t4 | char(r >> 18);
	s[1] = tx | (char(r >> 12) & maskx);
	s[2] = tx | (char(r >> 6) & maskx);
	s[3] = tx | (char(r) & maskx);
	return 4;
}

int rune_count(slice<const char> s) {
	int n = 0, i = 0;
	while (i < s.len()) {
		if (uint8(s[i]) < rune_self) {
			i++;
		} else {
			i += decode_rune(s.sub(i)).size;
		}
		n++;
	}
	return n;
}

bool rune_start(char b) {
	return (b & 0xC0) != 0x80;
}

bool valid(slice<const char> s) {
	int i = 0;
	while (i < s.len()) {
		if (uint8(s[i]) < rune_self) {
			i++;
		} else {
			int size = decode_rune(s.sub(i)).size;
			if (size == 1) {
				// All valid runes of size 1 (those
				// below rune_self) were handled above.
				// This must be a rune_error.
				return false;
			}
			i += size;
		}
	}
	return true;
}

bool valid_rune(rune r) {
	if (r < 0) {
		return false;
	} else if (surrogate_min <= r && r <= surrogate_max) {
		return false;
	} else if (r > max_rune) {
		return false;
	}
	return true;
}

}}} // namespace zbs::unicode::utf8
