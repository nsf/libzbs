#include "zbs/_map.hh"
#include "zbs/_slice.hh"

namespace zbs {

int hash<const char*>::operator()(const char *s, int seed) {
	return hash<slice<const char>>()(s, seed);
}

} // namespace zbs
