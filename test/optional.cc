#include "stf.hh"
#include "zbs.hh"

STF_SUITE_NAME("zbs::optional");

class oop {
public:
	static int balance;

	oop() { balance++; }
	oop(const oop&) { balance++; }
	oop(oop&&) { balance++; }
	~oop() { balance--; }

	oop &operator=(const oop&) = default;
};

int oop::balance = 0;

using namespace zbs;

STF_TEST("random") {
	optional<int> x;
	STF_ASSERT(static_cast<bool>(x) == false);
	x = 10;
	STF_ASSERT(static_cast<bool>(x) == true);
	STF_ASSERT(x == 10);
	STF_ASSERT(10 == x);
	STF_ASSERT(!(x == 7));
	STF_ASSERT(!(7 == x));
	STF_ASSERT(*x == 10);
	STF_ASSERT(*x != -5);
	x = nullopt;
	STF_ASSERT(static_cast<bool>(x) == false);
	STF_ASSERT(!(x == 10));
	STF_ASSERT(!(10 == x));
	STF_ASSERT(!(x == 7));
	STF_ASSERT(!(7 == x));

	optional<oop> y;
	y = oop{};
	optional<oop> z = y;
	y = z;
	z = nullopt;
	y = z;
	z = oop{};
	y = z;
}

STF_TEST("oop ctor/dtor balance correctness") {
	STF_ASSERT(oop::balance == 0);
}
