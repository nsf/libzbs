// This file may contain portions of the code derived/adopted from the Go
// standard library, which is covered by a BSD-style license. You can find more
// details in 3rdparty/go_license.txt file.

#pragma once

#include "../types.hh"
#include "../slice.hh"

namespace zbs {
namespace unicode {
namespace utf8 {

/// The "error" rune or a "Unicode replacement character"
const rune rune_error = U'\uFFFD';

/// Characters below `rune_self` are represented as themselves in a single byte
const rune rune_self = 0x80;

/// Maximum valid Unicode code point
const rune max_rune = U'\U0010FFFF';

/// Maximum number of bytes of a UTF-8 encoded Unicode character
const int utf_max = 4;

rune decode_last_rune(slice<const char> s, int *size = nullptr);
rune decode_rune(slice<const char> s, int *size = nullptr);
int encode_rune(slice<char> s, rune r);
bool full_rune(slice<const char> s);
int rune_count(slice<const char> s);
int rune_len(rune r);
bool rune_start(char b);
bool valid(slice<const char> s);
bool valid_rune(rune r);

// TODO: Go has some built-in utf8 functionality in the language like string ->
// []rune conversions. We need these too in some form here or should we
// implement them as u32string?

}}} // namespace zbs::unicode::utf8
