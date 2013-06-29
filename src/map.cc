#include "zbs/_map.hh"

namespace zbs {

int hash<const char*>::operator()(const char *s, int seed) {
	constexpr unsigned M0 = 2860486313U;
	constexpr unsigned M1 = 3267000013U;
	unsigned hash = M0 ^ seed;
	while (*s)
		hash = (hash ^ *s++) * M1;
	return hash;
}

} // namespace zbs
