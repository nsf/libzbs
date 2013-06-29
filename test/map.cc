#include "stf.hh"
#include "zbs.hh"

STF_SUITE_NAME("zbs::map");

using namespace zbs;

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

/*
STF_TEST("map::map()") {
	map<string, string> a;
	for (int i = 0; i < 50; i++) {
		string k = fmt::sprintf("%d", i);
		string v = fmt::sprintf("%d%d", i, i);
		a[k] = v;
	}

	for (int i = 0; i < 50; i++) {
		string k = fmt::sprintf("%d", i);
		string v = fmt::sprintf("%d%d", i, i);
		STF_PRINTF("val: %s", a[k].c_str());
		STF_ASSERT(a[k] == v);
	}
	STF_ASSERT(a.len() == 50);

	map<string, oop> b(100);
	for (int i = 0; i < 50; i++) {
		string k = fmt::sprintf("item_%d", i);
		b[k];
	}
	STF_ASSERT(b.len() == 50);
}
*/

STF_TEST("map::map(std::initializer_list<key_and_value<K, V>>)") {
	map<string, string> a = {
		{"John Smith", "521-1234"},
		{"Lisa Smith", "521-8976"},
		{"Sandra Dee", "521-9655"},
		{"Ted Baker", "418-4165"},
		{"Sam Doe", "521-5030"},
	};
	STF_ASSERT(a.len() == 5);
	STF_ASSERT(a["John Smith"] == "521-1234");
	STF_ASSERT(a["Lisa Smith"] == "521-8976");
	STF_ASSERT(a["Sandra Dee"] == "521-9655");
	STF_ASSERT(a["Ted Baker"] == "418-4165");
	STF_ASSERT(a["Sam Doe"] == "521-5030");

	map<string, oop> b = {
		{"John Smith", oop()},
		{"Lisa Smith", oop()},
		{"Sandra Dee", oop()},
		{"Ted Baker", oop()},
		{"Sam Doe", oop()},
	};
	STF_ASSERT(b.len() == 5);
}

STF_TEST("map_iter") {
	map<string, string> a = {
		{"John Smith", "521-1234"},
		{"Lisa Smith", "521-8976"},
		{"Sandra Dee", "521-9655"},
		{"Ted Baker", "418-4165"},
		{"Sam Doe", "521-5030"},
	};
	int i = 0;
	for (const auto &it : a) {
		STF_PRINTF("%s: %s", it.key.c_str(), it.value.c_str());
		if (it.key == "Sandra Dee")
			it.value = "OOPS";
		i++;
	}
	STF_ASSERT(i == 5);
	STF_ASSERT(a["Sandra Dee"] == "OOPS");
}

STF_TEST("oop ctor/dtor balance correctness") {
	STF_ASSERT(oop::balance == 0);
}
