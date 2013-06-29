#pragma once

#include "_types.hh"
#include "_string.hh"
#include "_vector.hh"
#include "_func.hh"

namespace zbs {
namespace strings {

bool              contains(slice<const char> s, slice<const char> substr);
bool              contains_any(slice<const char> s, slice<const char> chars);
bool              contains_rune(slice<const char> s, rune r);
int               count(slice<const char> s, slice<const char> sep);
bool              equal_fold(slice<const char> a, slice<const char> b);
vector<string>    fields(slice<const char> s);
vector<string>    fields_func(slice<const char> s, func<bool(rune)> f);
bool              starts_with(slice<const char> s, slice<const char> prefix);
bool              ends_with(slice<const char> s, slice<const char> suffix);
int               index(slice<const char> s, slice<const char> sep);
int               index_any(slice<const char> s, slice<const char> chars);
int               index_func(slice<const char> s, func<bool(rune)> f);
int               index_rune(slice<const char> s, rune r);
string            join(slice<const string> a, slice<const char> sep);
int               last_index(slice<const char> s, slice<const char> sep);
int               last_index_any(slice<const char> s, slice<const char> chars);
int               last_index_func(slice<const char> s, func<bool(rune)> f);
string            map(func<rune(rune)> f, slice<const char> s);
string            repeat(slice<const char> s, int count);
string            replace(slice<const char> s, slice<const char> old, slice<const char> _new, int n);
vector<string>    split(slice<const char> s, slice<const char> sep);
vector<string>    split_after(slice<const char> s, slice<const char> sep);
vector<string>    split_after_n(slice<const char> s, slice<const char> sep, int n);
vector<string>    split_n(slice<const char> s, slice<const char> sep, int n);
string            title(slice<const char> s);
string            to_lower(slice<const char> s);
string            to_title(slice<const char> s);
string            to_upper(slice<const char> s);
//string          to_lower_special(unicode::special_case case, slice<const char> s);
//string          to_title_special(unicode::special_case case, slice<const char> s);
//string          to_upper_special(unicode::special_case case, slice<const char> s);
slice<const char> trim(slice<const char> s, slice<const char> cutset);
slice<const char> trim_func(slice<const char> s, func<bool(rune)> f);
slice<const char> trim_left(slice<const char> s, slice<const char> cutset);
slice<const char> trim_left_func(slice<const char> s, func<bool(rune)> f);
slice<const char> trim_right(slice<const char> s, slice<const char> cutset);
slice<const char> trim_right_func(slice<const char> s, func<bool(rune)> f);
slice<const char> trim_space(slice<const char> s);
slice<const char> trim_prefix(slice<const char> s, slice<const char> prefix);
slice<const char> trim_suffix(slice<const char> s, slice<const char> suffix);

}} // namespace zbs::strings
