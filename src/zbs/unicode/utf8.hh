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

/// Unpacks the last UTF-8 encoding in `s` and returns the rune and its width
/// in bytes. If the encoding is invalid, it returns (#rune_error, 1), an
/// impossible result for correct UTF-8. An encoding is invalid if it is
/// incorrect UTF-8, encodes a rune that is out of range, or is not the
/// shortest possible UTF-8 encoding for the value. No other validation is
/// performed.
rune decode_last_rune(slice<const char> s, int *size = nullptr);

/// Unpacks the first UTF-8 encoding in `s` and returns the rune and its width
/// in bytes. If the encoding is invalid, it returns (#rune_error, 1), an
/// impossible result for correct UTF-8. An encoding is invalid if it is
/// incorrect UTF-8, encodes a rune that is out of range, or is not the
/// shortest possible UTF-8 encoding for the value. No other validation is
/// performed.
rune decode_rune(slice<const char> s, int *size = nullptr);

/// Writes into `s` (which must be large enough) the UTF-8 encoding for the
/// rune. It returns the number of bytes written.
int encode_rune(slice<char> s, rune r);

/// Reports whether the bytes in `s` begin with a full UTF-8 encoding of a
/// rune. An invalid encoding is considered a full rune since it will convert
/// as a width-1 error rune.
bool full_rune(slice<const char> s);

/// Returns the number of runes in `s`. Erroneous and short encodings are
/// treated as single runes of width 1 byte.
int rune_count(slice<const char> s);

/// Returns the number of bytes required to encode the rune. It returns -1 if
/// the rune is not a valid value to encode in UTF-8.
int rune_len(rune r);

/// Reports whether the byte could be the first byte of an encoded rune. Second
/// and subsequent bytes always have the top two bits set to 10.
bool rune_start(char b);

/// Reports whether `s` consists entirely of valid UTF-8 encoded runes.
bool valid(slice<const char> s);

/// Reports whether `r` can be legally encoded as UTF-8. Code points that are
/// out of range or surrogate half are illegal.
bool valid_rune(rune r);

// TODO: Go has some built-in utf8 functionality in the language like string ->
// []rune conversions. We need these too in some form here or should we
// implement them as u32string?

}}} // namespace zbs::unicode::utf8
