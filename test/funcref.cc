#include "stf.hh"
#include "zbs.hh"

STF_SUITE_NAME("zbs::funcref");

static int eval(zbs::funcref<int (int, int)> f, int a, int b) {
	return f(a, b);
}

static int double_eval(zbs::funcref<int (int, int)> f, int a, int b) {
	return eval(f, a, b);
}

static int func_adder(int a, int b) {
	return a + b;
}

STF_TEST("funcref::funcref(T&)") {
	STF_ASSERT(eval([](int a, int b){ return a + b; }, 5, 10) == 15);
	STF_ASSERT(double_eval([](int a, int b){ return a + b; }, 5, 10) == 15);
	int xa = 1;
	int xb = 2;
	STF_ASSERT(eval([=](int, int){ return xa + xb; }, -1, -1) == 3);
}

STF_TEST("funcref::funcref(R (*)(Args...))") {
	STF_ASSERT(eval(func_adder, -5, -10) == -15);
	STF_ASSERT(double_eval(func_adder, -5, -10) == -15);
}
