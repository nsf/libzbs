#include "stf.hh"
#include "zbs/vector.hh"
#include <utility>

STF_SUITE_NAME("zbs/vector");

bool check_vector(zbs::vector<int> &v, const zbs::vector<int> &gold) {
	if (v.len() != gold.len()) {
		return false;
	}
	// sanity check
	if (v.cap() < gold.len()) {
		return false;
	}

	for (int i = 0; i < v.len(); i++) {
		if (v[i] != gold[i]) {
			return false;
		}
	}
	return true;
}

class oop {
public:
	static int balance;

	static int default_ctors;
	static int copy_ctors;
	static int move_ctors;
	static int copy_ops;
	static int move_ops;
	static int dtors;

	static void reset_global_counters() {
		default_ctors = 0;
		copy_ctors = 0;
		move_ctors = 0;
		copy_ops = 0;
		move_ops = 0;
		dtors = 0;
	}

	static int all_ctors() {
		return default_ctors + copy_ctors + move_ctors;
	}

	static int all_ops() {
		return copy_ops + move_ops;
	}

	oop() { default_ctors++; balance++; }
	oop(const oop &r) { copy_ctors++; balance++; }
	oop(oop &&r) { move_ctors++; balance++; }
	~oop() { dtors++; balance--; }
	oop &operator=(const oop &r) { copy_ops++; return *this; }
	oop &operator=(oop &&r) { move_ops++; return *this; }
};

int oop::balance = 0;

int oop::default_ctors = 0;
int oop::copy_ctors = 0;
int oop::move_ctors = 0;
int oop::copy_ops = 0;
int oop::move_ops = 0;
int oop::dtors = 0;

STF_TEST("vector::vector()") {
	// default ctor
	zbs::vector<int> a;
	STF_ASSERT(a.len() == 0);
	STF_ASSERT(a.cap() == 0);
	STF_ASSERT(a.data() == nullptr);
}

STF_TEST("vector::vector(std::initializer_list<T>)") {
	// initializer list ctor
	zbs::vector<int> a = {3, 4, 5};
	STF_ASSERT(check_vector(a, {3, 4, 5}));
}

STF_TEST("vector::vector(const vector&)") {
	// copy ctor
	zbs::vector<int> a = {3, 4, 5};
	zbs::vector<int> b = a;
	STF_ASSERT(check_vector(b, {3, 4, 5}));
}

STF_TEST("vector::vector(vector&&)") {
	// move ctor
	zbs::vector<int> a = {3, 4, 5};
	zbs::vector<int> b = std::move(a);
	STF_ASSERT(a.len() == 0);
	STF_ASSERT(a.cap() == 0);
	STF_ASSERT(a.data() == nullptr);
	STF_ASSERT(check_vector(b, {3, 4, 5}));
}

STF_TEST("vector::vector(slice<const T>)") {
	// slice ctor
	zbs::vector<int> a = {7, 5, 10, 12};
	zbs::vector<int> b = a.sub(1, 3);
	STF_ASSERT(check_vector(b, {5, 10}));

	zbs::vector<int> c = a.sub(0, 0);
	STF_ASSERT(c.len() == 0);
	STF_ASSERT(c.cap() == 0);
	STF_ASSERT(c.data() == nullptr);

	// slice ctor (oop)
	oop::reset_global_counters();
	zbs::vector<oop> d(3);
	zbs::vector<oop> e = d.sub(0, 2);
	STF_ASSERT(e.len() == 2);
	STF_ASSERT(e.cap() >= 2);
	STF_ASSERT(oop::default_ctors == 3);
	STF_ASSERT(oop::copy_ctors == 2);
	STF_ASSERT(oop::all_ctors() == 5);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 0);
}

STF_TEST("vector::vector(int)") {
	// size ctor
	zbs::vector<int> a(10);
	STF_ASSERT(a.len() == 10);
	STF_ASSERT(a.cap() >= 10);
	STF_ASSERT(a.data() != nullptr);

	zbs::vector<int> b(0);
	STF_ASSERT(b.len() == 0);
	STF_ASSERT(b.cap() == 0);
	STF_ASSERT(b.data() == nullptr);
}

STF_TEST("vector::vector(int, const T&)") {
	// size factory ctor
	zbs::vector<int> a(5, -1);
	STF_ASSERT(check_vector(a, {-1, -1, -1, -1, -1}));

	zbs::vector<int> b(0, -1);
	STF_ASSERT(b.len() == 0);
	STF_ASSERT(b.cap() == 0);
	STF_ASSERT(b.data() == nullptr);
}

STF_TEST("vector::operator=(slice<const T>)") {
	// slice operator=
	zbs::vector<int> a = {3, 4, 9};
	a = a.sub(1, 2);
	STF_ASSERT(check_vector(a, {4}));

	zbs::vector<int> b = {7, 20, 40, -1};
	b = b.sub(0, 2);
	STF_ASSERT(check_vector(b, {7, 20}));
}

STF_TEST("vector::operator=(slice<const T>) [oop]") {
	// self-slicing
	oop::reset_global_counters();
	zbs::vector<oop> a(4);
	a = a.sub(1, 3);
	STF_ASSERT(a.len() == 2);
	STF_ASSERT(a.cap() >= 2);
	STF_ASSERT(oop::default_ctors == 4);
	STF_ASSERT(oop::all_ctors() == 4);
	STF_ASSERT(oop::copy_ops == 2);
	STF_ASSERT(oop::all_ops() == 2);
	STF_ASSERT(oop::dtors == 2);

	// self-assignment
	oop::reset_global_counters();
	zbs::vector<oop> b(4);
	b = b.sub();
	STF_ASSERT(b.len() == 4);
	STF_ASSERT(b.cap() >= 4);
	STF_ASSERT(oop::default_ctors == 4);
	STF_ASSERT(oop::all_ctors() == 4);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 0);

	// assign large slice to a small one
	oop::reset_global_counters();
	zbs::vector<oop> c(4);
	zbs::vector<oop> d(1);
	d = c.sub();
	STF_ASSERT(d.len() == 4);
	STF_ASSERT(d.cap() >= 4);
	STF_ASSERT(oop::default_ctors == 5);
	STF_ASSERT(oop::copy_ctors == 4);
	STF_ASSERT(oop::all_ctors() == 9);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 1);

	// assign slice to a slice with preallocated space
	oop::reset_global_counters();

	// (5 () ctor)
	zbs::vector<oop> e(4);
	zbs::vector<oop> f(1);

	// move f[0] to new f[0], destroy old (1 move ctor, 1 dtor)
	f.reserve(7);
	// assign e[0] to f[0] (1 copy op=), assign e[1..3] to f[1..3] (3 copy ctor)
	f = e.sub();
	STF_ASSERT(f.len() == 4);
	STF_ASSERT(f.cap() >= 4);
	STF_ASSERT(oop::default_ctors == 5);
	STF_ASSERT(oop::copy_ctors == 3);
	STF_ASSERT(oop::move_ctors == 1);
	STF_ASSERT(oop::all_ctors() == 9);
	STF_ASSERT(oop::copy_ops == 1);
	STF_ASSERT(oop::all_ops() == 1);
	STF_ASSERT(oop::dtors == 1);
}

STF_TEST("vector::operator=(std::initializer_list<T>)") {
	// initializer list operator=
	zbs::vector<int> a = {1, 2, 3, 4};
	a = {5, 9};
	STF_ASSERT(check_vector(a, {5, 9}));
}

STF_TEST("vector::operator=(const vector&)") {
	// copy operator=
	zbs::vector<int> a = {1, 5, 7, 12};
	zbs::vector<int> b = {20, 40};
	a = b;
	STF_ASSERT(check_vector(a, {20, 40}));
	STF_ASSERT(check_vector(b, {20, 40}));
	a = a;
	STF_ASSERT(check_vector(a, {20, 40}));
}

STF_TEST("vector::operator=(vector&&)") {
	// move operator=
	zbs::vector<int> a = {43, 56, 22};
	zbs::vector<int> b;
	b = std::move(a);
	STF_ASSERT(check_vector(b, {43, 56, 22}));
	STF_ASSERT(a.data() == nullptr);
	STF_ASSERT(a.cap() == 0);
	STF_ASSERT(check_vector(a, {}));
}

STF_TEST("vector::clear()") {
	// clear
	zbs::vector<int> a = {1, 2, 3};
	a.clear();
	STF_ASSERT(a.len() == 0);
	STF_ASSERT(a.cap() == 3);
	STF_ASSERT(a.data() != nullptr);

	// clear (oop)
	oop::reset_global_counters();
	zbs::vector<oop> b(4);
	b.clear();
	STF_ASSERT(b.len() == 0);
	STF_ASSERT(b.cap() != 0);
	STF_ASSERT(b.data() != nullptr);
	STF_ASSERT(oop::default_ctors == 4);
	STF_ASSERT(oop::all_ctors() == 4);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 4);
}

STF_TEST("vector::reserve(int)") {
	// reserve
	zbs::vector<int> a = {0, 5, 10};
	a.reserve(10);
	STF_ASSERT(a.cap() >= 10);
	STF_ASSERT(check_vector(a, {0, 5, 10}));

	// reserve (oop)
	oop::reset_global_counters();
	zbs::vector<oop> b(3);
	b.reserve(10);
	STF_ASSERT(b.cap() >= 10);
	STF_ASSERT(b.len() == 3);
	STF_ASSERT(oop::default_ctors == 3);
	STF_ASSERT(oop::move_ctors == 3);
	STF_ASSERT(oop::all_ctors() == 6);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 3);

	// negative reserve
	zbs::vector<int> c = {7, 4, 3};
	c.reserve(-10);
	STF_ASSERT(check_vector(c, {7, 4, 3}));
}

STF_TEST("vector::shrink()") {
	// shrink
	zbs::vector<int> a = {0, 5, 10};
	a.reserve(10);
	a.shrink();
	STF_ASSERT(a.cap() == 3);
	STF_ASSERT(check_vector(a, {0, 5, 10}));

	// shrink (oop)
	oop::reset_global_counters();
	zbs::vector<oop> b(3);
	b.reserve(10);
	b.shrink();
	STF_ASSERT(b.cap() == 3);
	STF_ASSERT(oop::default_ctors == 3);
	STF_ASSERT(oop::move_ctors == 6);
	STF_ASSERT(oop::all_ctors() == 9);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 6);

	// shrink (oop) [2]
	oop::reset_global_counters();
	zbs::vector<oop> c;
	c.reserve(10);
	c.shrink();
	STF_ASSERT(c.cap() == 0);
	STF_ASSERT(c.len() == 0);
	STF_ASSERT(c.data() == nullptr);
	STF_ASSERT(oop::all_ctors() == 0);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 0);
}

STF_TEST("vector::resize(int)") {
	// resize
	zbs::vector<int> a;
	a.resize(3);
	STF_ASSERT(a.len() == 3);

	zbs::vector<int> b = {-1, -2, -3};
	b.resize(1);
	STF_ASSERT(b.cap() >= 3);
	STF_ASSERT(check_vector(b, {-1}));

	zbs::vector<int> c = {1, 2};
	c.resize(5, 3);
	STF_ASSERT(check_vector(c, {1, 2, 3, 3, 3}));

	// resize (oop)
	oop::reset_global_counters();
	zbs::vector<oop> d;
	d.resize(3);
	STF_ASSERT(oop::default_ctors == 3);
	STF_ASSERT(oop::all_ctors() == 3);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 0);

	oop::reset_global_counters();
	d.resize(1);
	STF_ASSERT(oop::all_ctors() == 0);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 2);

	oop::reset_global_counters();
	d.resize(0);
	STF_ASSERT(oop::all_ctors() == 0);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 1);
	STF_ASSERT(d.len() == 0);
	STF_ASSERT(d.cap() > 0);
	STF_ASSERT(d.data() != nullptr);
}

STF_TEST("vector::insert(int, const T&)") {
	// at the beginning
	zbs::vector<int> a = {1, 2, 3};
	a.insert(0, 0);
	STF_ASSERT(check_vector(a, {0, 1, 2, 3}));

	// in the middle
	zbs::vector<int> b = {1, 2, 3};
	b.insert(2, 333);
	STF_ASSERT(check_vector(b, {1, 2, 333, 3}));

	// at the end
	zbs::vector<int> c = {7, 8, 9};
	c.insert(c.len(), 10);
	STF_ASSERT(check_vector(c, {7, 8, 9, 10}));
}

STF_TEST("vector::insert(int, slice<const T>)") {
	// everywhere, foreign slice insertion
	zbs::vector<int> a = {10, -10};
	zbs::vector<int> b = {1, 2, 3};
	b.insert(0, a);
	STF_ASSERT(check_vector(b, {10, -10, 1, 2, 3}));
	b.insert(4, a);
	STF_ASSERT(check_vector(b, {10, -10, 1, 2, 10, -10, 3}));
	b.insert(2, a);
	STF_ASSERT(check_vector(b, {10, -10, 10, -10, 1, 2, 10, -10, 3}));
	b.insert(b.len(), a);
	STF_ASSERT(check_vector(b, {10, -10, 10, -10, 1, 2, 10, -10, 3, 10, -10}));

	// inserting self
	zbs::vector<int> c = {1, 2, 3, 4, 5};
	c.insert(0, c.sub(3));
	STF_ASSERT(check_vector(c, {4, 5, 1, 2, 3, 4, 5}));
	c.insert(c.len(), c.sub(2, 4));
	STF_ASSERT(check_vector(c, {4, 5, 1, 2, 3, 4, 5, 1, 2}));

	// cutting insertion
	c.insert(2, c.sub(0, 4));
	STF_ASSERT(check_vector(c, {4, 5, 4, 5, 1, 2, 1, 2, 3, 4, 5, 1, 2}));

	// empty insertion
	zbs::vector<int> d = {7, 8, 9, 10};
	d.insert(0, zbs::slice<int>());
	STF_ASSERT(check_vector(d, {7, 8, 9, 10}));
}

STF_TEST("vector::append(const T&)") {
	zbs::vector<int> a;
	for (int i = 0; i < 10; i++) {
		a.append(i);
	}
	STF_ASSERT(check_vector(a, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

STF_TEST("vector::append(slice<const T>)") {
	zbs::vector<int> a = {1, 2};
	zbs::vector<int> b = {10, 20, 30, 40, 50};
	a.append(b.sub(1, 3));
	STF_ASSERT(check_vector(a, {1, 2, 20, 30}));

	// append self slice
	zbs::vector<int> c = {5, 20, -40};
	c.append(c);
	STF_ASSERT(check_vector(c, {5, 20, -40, 5, 20, -40}));
}

STF_TEST("vector::remove(int)") {
	zbs::vector<int> a = {1, 2, 3};
	a.remove(0);
	STF_ASSERT(check_vector(a, {2, 3}));
	a.remove(1);
	STF_ASSERT(check_vector(a, {2}));

	// remove (oop)
	oop::reset_global_counters();
	zbs::vector<oop> b(4);
	b.remove(1);
	STF_ASSERT(oop::default_ctors == 4);
	STF_ASSERT(oop::move_ctors == 2);
	STF_ASSERT(oop::all_ctors() == 6);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 3);
}

STF_TEST("vector::remove(int, int)") {
	zbs::vector<int> a = {1, 2, 3, 4, 5, 6};
	a.remove(4, 6);
	STF_ASSERT(check_vector(a, {1, 2, 3, 4}));
	a.remove(1, 3);
	STF_ASSERT(check_vector(a, {1, 4}));
	a.remove(0, 2);
	STF_ASSERT(check_vector(a, {}));

	// remove slice (oop)
	oop::reset_global_counters();
	zbs::vector<oop> b(3);
	b.remove(0, 2);
	STF_ASSERT(oop::default_ctors == 3);
	STF_ASSERT(oop::move_ctors == 1);
	STF_ASSERT(oop::all_ctors() == 4);
	STF_ASSERT(oop::all_ops() == 0);
	STF_ASSERT(oop::dtors == 3);
}

STF_TEST("oop ctor/dtor balance correctness") {
	STF_ASSERT(oop::balance == 0);
}
