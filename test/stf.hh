#pragma once

// simple testing framework
// TODO: manual

#include <vector>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <chrono>

#define _CC(a, b) a ## b
#define _MCC(a, b) _CC(a, b)

#define STF_TEST(name)										\
static void _MCC(_test_func_, __LINE__)(stf::runner&, stf::test&);				\
static stf::test _MCC(_test_, __LINE__)(_stf_runner, name, _MCC(_test_func_, __LINE__));	\
static void _MCC(_test_func_, __LINE__)(stf::runner &__R, stf::test &__T)

#define STF_SUITE_NAME(name)									\
static stf::name_setter _MCC(_name_setter_, __LINE__)(_stf_runner, name);

#define STF_LOGF(...) __R.logf(__LINE__, __FILE__, __VA_ARGS__)
#define STF_RAW_LOGF(...) __R.logf(0, nullptr, __VA_ARGS__)
#define STF_PRINTF(...) __R.printf(__LINE__, __FILE__, __VA_ARGS__)
#define STF_RAW_PRINTF(...) __R.printf(0, nullptr, __VA_ARGS__)
#define STF_ERRORF(...)										\
do {												\
	__R.printf(__LINE__, __FILE__, __VA_ARGS__);						\
	__T.status = false;									\
} while (0)

#define STF_ASSERT(expr)									\
do {												\
	if (!(expr)) {										\
		__R.printf(__LINE__, __FILE__, "assertion failed: %s", #expr);			\
		__T.status = false;								\
	}											\
} while (0)

namespace stf {

struct runner;
struct test;

typedef void (*functype)(runner&, test&);

struct test {
	std::string name;
	functype func;
	bool status = true;

	test(runner &r, std::string name, functype);
};

struct name_setter {
	name_setter(runner &r, std::string name);
};

struct runner {
	std::string suite_name;
	std::vector<test> tests;
	int failed = 0;

	// copy & paste from logf, without verbosity check
	void printf(int line, const char *filename, const char *format, ...) {
		if (filename != nullptr) {
			fprintf(stderr, "%s:%d: ", filename, line);
		}

		va_list vl;
		va_start(vl, format);
		vfprintf(stderr, format, vl);
		va_end(vl);

		fprintf(stderr, "\n");
	}

	int run() {
		runner &__R = *this; // for macros
		auto start = std::chrono::system_clock::now();
		for (auto &t: tests) {
			STF_RAW_PRINTF("=== Running %s", t.name.c_str());
			(*t.func)(*this, t);
			if (t.status) {
				STF_RAW_PRINTF("--- PASS");
			} else {
				failed++;
				STF_RAW_PRINTF("--- FAIL");
			}
		}
		auto end = std::chrono::system_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
			(end-start).count();
		auto s_part = ms / 1000;
		auto ms_part = ms - s_part * 1000;
		if (failed > 0) {
			STF_RAW_PRINTF("%d out of %d test(s) failed", failed, tests.size());
		}

		if (failed > 0) {
			STF_RAW_PRINTF("FAIL\t%s\t%d.%03ds", suite_name.c_str(), s_part, ms_part);
			return 1;
		} else {
			STF_RAW_PRINTF("ok\t%s\t%d.%03ds", suite_name.c_str(), s_part, ms_part);
			return 0;
		}
	}
};

test::test(runner &r, std::string name, functype func): name(name), func(func) {
	r.tests.push_back(*this);
}

name_setter::name_setter(runner &r, std::string name) {
	r.suite_name = name;
}

} // namespace stf

static stf::runner _stf_runner;
int main(int argc, char **argv) {
	return _stf_runner.run();
}
