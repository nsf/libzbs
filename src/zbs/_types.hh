#pragma once

#include <cstdint>

namespace zbs {

/// @defgroup basictypes Basic typedefs.
/// @addtogroup basictypes
/// @{

typedef uint8_t  uint8;  ///< Unsigned integer type with width of 8 bits.
typedef uint16_t uint16; ///< Unsigned integer type with width of 16 bits.
typedef uint32_t uint32; ///< Unsigned integer type with width of 32 bits.
typedef uint64_t uint64; ///< Unsigned integer type with width of 64 bits.
typedef int8_t   int8;   ///< Integer type with width of 8 bits.
typedef int16_t  int16;  ///< Integer type with width of 16 bits.
typedef int32_t  int32;  ///< Integer type with width of 32 bits.
typedef int64_t  int64;  ///< Integer type with width of 64 bits.
typedef uint8    byte;   ///< Convenience alias for uint8.
typedef int32    rune;   ///< Unicode code point.

/// @}

/// Rune and offset pair type. Has `rune` and `offset` fields. Used in
/// string_iter.
/// @headerfile zbs.hh
struct offset_rune {
	zbs::rune rune;
	int offset;
};

/// Rune and size pair type. Has `rune` and `size` fields. Used in utf8 module.
/// @headerfile zbs.hh
struct sized_rune {
	zbs::rune rune;
	int size;
};

} // namespace zbs
