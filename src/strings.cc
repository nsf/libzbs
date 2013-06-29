// This file may contain portions of the code derived/adopted from the Go
// standard library, which is covered by a BSD-style license. You can find more
// details in 3rdparty/go_license.txt file.

#include "zbs/strings.hh"

#include <utility>
#include "zbs/slices.hh"
#include "zbs/unicode.hh"
#include "zbs/unicode/utf8.hh"

namespace unicode = zbs::unicode;
namespace utf8 = zbs::unicode::utf8;

namespace zbs {
namespace strings {

bool contains(slice<const char> s, slice<const char> substr) {
	return slices::contains(s, substr);
}

bool contains_any(slice<const char> s, slice<const char> chars) {
	return index_any(s, chars) >= 0;
}

bool contains_rune(slice<const char> s, rune r) {
	return index_rune(s, r) >= 0;
}

int count(slice<const char> s, slice<const char> sep) {
	if (sep.len() == 0) {
		return utf8::rune_count(s) + 1;
	}
	return slices::count(s, sep);
}

bool equal_fold(slice<const char> a, slice<const char> b) {
	while (a != "" && b != "") {
		// extract first rune from each string
		rune ar, br;
		if (uint8(a[0]) < utf8::rune_self) {
			ar = a[0];
			a = a.sub(1);
		} else {
			int size;
			ar = utf8::decode_rune(a, &size);
			a = a.sub(size);
		}
		if (uint8(b[0]) < utf8::rune_self) {
			br = b[0];
			b = b.sub(1);
		} else {
			int size;
			br = utf8::decode_rune(b, &size);
			b = b.sub(size);
		}

		// if they match, keep going; if not, return false

		// easy case
		if (ar == br) {
			continue;
		}

		// make ar < br to simplify what follows
		if (br < ar) {
			std::swap(ar, br);
		}

		// fast check for ASCII
		if (br < utf8::rune_self && 'A' <= ar && ar <= 'Z') {
			// ASCII, and ar is upper case; br must be lower case
			if (br == ar + ('a' - 'A')) {
				continue;
			}
			return false;
		}

		// General case. simple_fold(x) returns the next equivalent
		// rune > x or wraps around to smaller values.
		rune r = unicode::simple_fold(ar);
		while (r != ar && r < br) {
			r = unicode::simple_fold(r);
		}
		if (r == br) {
			continue;
		}
		return false;
	}

	// one string is empty, maybe both are empty
	return a == b;
}

vector<string> fields(slice<const char> s) {
	return fields_func(s, unicode::is_space);
}

vector<string> fields_func(slice<const char> s, func<bool(rune)> f) {
	int n = 0;
	bool in_field = false;
	for (const auto &it : string_iter(s)) {
		bool was_in_field = in_field;
		in_field = !f(it.rune);
		if (in_field && !was_in_field) {
			n++;
		}
	}

	vector<string> a;
	a.reserve(n);

	int field_start = -1;
	for (const auto &it : string_iter(s)) {
		if (f(it.rune)) {
			if (field_start >= 0) {
				a.append(s.sub(field_start, it.offset));
				field_start = -1;
			}
		} else if (field_start == -1) {
			field_start = it.offset;
		}
	}
	if (field_start >= 0) {
		// last field might end at EOF
		a.append(s.sub(field_start));
	}
	return a;
}

bool starts_with(slice<const char> s, slice<const char> prefix) {
	return slices::starts_with(s, prefix);
}

bool ends_with(slice<const char> s, slice<const char> suffix) {
	return slices::ends_with(s, suffix);
}

int index(slice<const char> s, slice<const char> sep) {
	return slices::index(s, sep);
}

int index_any(slice<const char> s, slice<const char> chars) {
	if (chars.len() == 0) {
		return -1;
	}

	for (const auto &it : string_iter(s)) {
		for (const auto &it2 : string_iter(chars)) {
			if (it.rune == it2.rune) {
				return it.offset;
			}
		}
	}
	return -1;
}

static int index_func_internal(slice<const char> s, func<bool(rune)> f, bool truth) {
	int start = 0;
	while (start < s.len()) {
		int wid = 1;
		rune r = s[start];
		if (uint8(r) >= utf8::rune_self) {
			r = utf8::decode_rune(s.sub(start), &wid);
		}
		if (f(r) == truth) {
			return start;
		}
		start += wid;
	}
	return -1;
}

int index_func(slice<const char> s, func<bool(rune)> f) {
	return index_func_internal(s, f, true);
}

int index_rune(slice<const char> s, rune r) {
	if (r < 0x80) {
		char c = r;
		for (int i = 0; i < s.len(); i++) {
			if (s[i] == c)
				return i;
		}
	} else {
		for (const auto &it : string_iter(s)) {
			if (it.rune == r)
				return it.offset;
		}
	}
	return -1;
}

string join(slice<const string> a, slice<const char> sep) {
	if (a.len() == 0) {
		return "";
	}
	if (a.len() == 1) {
		return a[0];
	}
	int n = sep.len() * (a.len() - 1);
	for (const auto &s : a) {
		n += s.len();
	}

	string out;
	out.reserve(n);

	out.append(a[0]);
	for (const auto &s : a.sub(1)) {
		out.append(sep);
		out.append(s);
	}

	return out;
}

int last_index(slice<const char> s, slice<const char> sep) {
	return slices::last_index(s, sep);
}

int last_index_any(slice<const char> s, slice<const char> chars) {
	if (chars.len() == 0) {
		return -1;
	}

	// TODO: use string_reverse_iter when it's ready
	for (int i = s.len(); i > 0;) {
		int size;
		rune r = utf8::decode_last_rune(s.sub(0, i), &size);
		i -= size;
		for (const auto &it : string_iter(chars)) {
			if (r == it.rune) {
				return i;
			}
		}
	}
	return -1;
}

static int last_index_func_internal(slice<const char> s, func<bool(rune)> f, bool truth) {
	// TODO: use string_reverse_iter when it's ready
	for (int i = s.len(); i > 0;) {
		int size;
		rune r = utf8::decode_last_rune(s.sub(0, i), &size);
		i -= size;
		if (f(r) == truth) {
			return i;
		}
	}
	return -1;
}

int last_index_func(slice<const char> s, func<bool(rune)> f) {
	return last_index_func_internal(s, f, true);
}

string map(func<rune(rune)> f, slice<const char> s) {
	string out;
	out.reserve(s.len());

	for (const auto &it : string_iter(s)) {
		rune r = f(it.rune);
		if (r >= 0) {
			char tmp[utf8::utf_max];
			int n = utf8::encode_rune(tmp, r);
			out.append(slice<char>(tmp).sub(0, n));
		}
	}
	return out;
}

string repeat(slice<const char> s, int count) {
	string ret;
	ret.reserve(s.len() * count);
	for (int i = 0; i < count; i++) {
		ret.append(s);
	}
	return ret;
}

string replace(slice<const char> s, slice<const char> old, slice<const char> _new, int n) {
	if (old == _new || n == 0) {
		return s;
	}

	// compute number of replacements
	int m = count(s, old);
	if (m == 0) {
		return s;
	} else if (n < 0 || m < n) {
		n = m;
	}

	string out;
	out.reserve(s.len()+n*(_new.len()-old.len()));

	int start = 0;
	for (int i = 0; i < n; i++) {
		int j = start;
		if (old.len() == 0) {
			if (i > 0) {
				int size;
				utf8::decode_rune(s.sub(start), &size);
				j += size;
			}
		} else {
			j += index(s.sub(start), old);
		}
		out.append(s.sub(start, j));
		out.append(_new);
		start = j + old.len();
	}
	out.append(s.sub(start));
	return out;
}

static vector<string> explode(slice<const char> s, int n) {
	if (n == 0) {
		return {};
	}
	int l = utf8::rune_count(s);
	if (n <= 0 || n > l) {
		n = l;
	}

	vector<string> out;
	out.reserve(n);

	int cur = 0;
	for (int i = 0; i < n-1; i++) {
		int size;
		rune ch = utf8::decode_rune(s.sub(cur), &size);
		if (ch == utf8::rune_error) {
			out.append("\uFFFD");
		} else {
			out.append(s.sub(cur, cur+size));
		}
		cur += size;
	}
	if (cur < s.len()) {
		out.append(s.sub(cur));
	}
	return out;
}

static vector<string> generic_split(slice<const char> s, slice<const char> sep,
	int sep_save, int n)
{
	if (n == 0) {
		return {};
	}
	if (sep == "") {
		return explode(s, n);
	}
	if (n < 0) {
		n = count(s, sep) + 1;
	}

	vector<string> out;
	char c = sep[0];
	int start = 0;
	for (int i = 0; i+sep.len() <= s.len() && out.len() < n-1; i++) {
		if (s[i] == c && (sep.len() == 1 || s.sub(i, i+sep.len()) == sep)) {
			out.append(s.sub(start, i+sep_save));
			start = i + sep.len();
			i += sep.len() - 1;
		}
	}
	out.append(s.sub(start));
	return out;
}

vector<string> split(slice<const char> s, slice<const char> sep) {
	return generic_split(s, sep, 0, -1);
}

vector<string> split_after(slice<const char> s, slice<const char> sep) {
	return generic_split(s, sep, sep.len(), -1);
}

vector<string> split_after_n(slice<const char> s, slice<const char> sep, int n) {
	return generic_split(s, sep, sep.len(), n);
}

vector<string> split_n(slice<const char> s, slice<const char> sep, int n) {
	return generic_split(s, sep, 0, n);
}

static bool is_separator(rune r) {
	if (r <= 0x7F) {
		if (
			('0' <= r && r <= '9') ||
			('a' <= r && r <= 'z') ||
			('A' <= r && r <= 'Z') ||
			r == '_'
		) {
			return false;
		}
		return true;
	}

	if (unicode::is_letter(r) || unicode::is_digit(r)) {
		return false;
	}
	return unicode::is_space(r);
}

string title(slice<const char> s) {
	// Use a lambda here to remember state. Hackish but effective. Depends
	// on map scanning in order and calling the lambda once per rune.
	rune prev = ' ';
	return map([&](rune r) {
		if (is_separator(prev)) {
			prev = r;
			return unicode::to_title(r);
		}
		prev = r;
		return r;
	}, s);
}

string to_lower(slice<const char> s) {
	return map(unicode::to_lower, s);
}

string to_title(slice<const char> s) {
	return map(unicode::to_title, s);
}

string to_upper(slice<const char> s) {
	return map(unicode::to_upper, s);
}

//string to_lower_special(unicode::special_case case, slice<const char> s);
//string to_title_special(unicode::special_case case, slice<const char> s);
//string to_upper_special(unicode::special_case case, slice<const char> s);

slice<const char> trim(slice<const char> s, slice<const char> cutset) {
	if (s == "" || cutset == "") {
		return s;
	}
	auto f = [=](rune r) { return contains_rune(cutset, r); };
	return trim_func(s, f);
}

slice<const char> trim_func(slice<const char> s, func<bool(rune)> f) {
	return trim_right_func(trim_left_func(s, f), f);
}

slice<const char> trim_left(slice<const char> s, slice<const char> cutset) {
	if (s == "" || cutset == "") {
		return s;
	}
	auto f = [=](rune r) { return contains_rune(cutset, r); };
	return trim_left_func(s, f);
}

slice<const char> trim_left_func(slice<const char> s, func<bool(rune)> f) {
	int i = index_func_internal(s, f, false);
	if (i == -1) {
		return {};
	}
	return s.sub(i);
}

slice<const char> trim_right(slice<const char> s, slice<const char> cutset) {
	if (s == "" || cutset == "") {
		return s;
	}
	auto f = [=](rune r) { return contains_rune(cutset, r); };
	return trim_right_func(s, f);
}

slice<const char> trim_right_func(slice<const char> s, func<bool(rune)> f) {
	int i = last_index_func_internal(s, f, false);
	if (i >= 0 && uint8(s[i]) >= utf8::rune_self) {
		int wid;
		utf8::decode_rune(s.sub(i), &wid);
		i += wid;
	} else {
		i++;
	}
	return s.sub(0, i);
}

slice<const char> trim_space(slice<const char> s) {
	return trim_func(s, unicode::is_space);
}

slice<const char> trim_prefix(slice<const char> s, slice<const char> prefix) {
	if (starts_with(s, prefix)) {
		return s.sub(prefix.len());
	}
	return s;
}

slice<const char> trim_suffix(slice<const char> s, slice<const char> suffix) {
	if (ends_with(s, suffix)) {
		return s.sub(0, s.len()-suffix.len());
	}
	return s;
}

}} // namespace zbs::strings
