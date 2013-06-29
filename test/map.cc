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

class move_only_str {
public:
	string str;

	move_only_str(const char *r): str(r) {}

	move_only_str() = delete;
	move_only_str(const move_only_str&) = delete;
	move_only_str(move_only_str&&) = default;
	move_only_str &operator=(const move_only_str&) = delete;
	move_only_str &operator=(move_only_str&&) = default;
	bool operator==(const move_only_str &r) const { return str == r.str; }
};

namespace zbs {

template <>
struct hash<move_only_str> {
	int operator()(const move_only_str &mos, int seed) {
		return hash<string>()(mos.str, seed);
	}
};

} // namespace zbs

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

STF_TEST("move semantics") {
	map<move_only_str, string> a;
	a["hello"] = "world";
	a["world"] = "hello";
	auto w = a.lookup("hello");
	STF_ASSERT(w != nullptr && *w == "world");
}

STF_TEST("map::lookup(const K&, V)") {
	map<string, int> m;
	m["a"] = 1;
	m["b"] = 2;
	m["c"] = 3;

	auto x = m.lookup("a", -1);
	STF_ASSERT(x == 1);
	auto y = m.lookup("b", -1);
	STF_ASSERT(y == 2);
	auto z = m.lookup("d", -1);
	STF_ASSERT(z == -1);
}

STF_TEST("oop ctor/dtor balance correctness") {
	STF_ASSERT(oop::balance == 0);
}
