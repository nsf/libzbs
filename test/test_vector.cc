#include "stf.hh"
#include "zbs/vector.hh"
#include <utility>

STF_SUITE_NAME("zbs/vector");

STF_TEST("vector ctor/dtor and ops") {
	{
		// default ctor
		zbs::vector<int> a;
		STF_ASSERT(a.len() == 0);
		STF_ASSERT(a.cap() == 0);
		STF_ASSERT(a.data() == nullptr);
	}

	{
		// initializer list ctor
		zbs::vector<int> a = {3, 4, 5};
		STF_ASSERT(a.len() == 3);
		STF_ASSERT(a.cap() == 3);
		STF_ASSERT(a[0] == 3 && a[1] == 4 && a[2] == 5);
	}

	{
		// copy ctor
		zbs::vector<int> a = {3, 4, 5};
		zbs::vector<int> b = a;
		STF_ASSERT(b.len() == 3);
		STF_ASSERT(b.cap() == 3);
		STF_ASSERT(b[0] == 3 && b[1] == 4 && b[2] == 5);
	}

	{
		// move ctor
		zbs::vector<int> a = {3, 4, 5};
		zbs::vector<int> b = std::move(a);
		STF_ASSERT(a.len() == 0);
		STF_ASSERT(a.cap() == 0);
		STF_ASSERT(a.data() == nullptr);
		STF_ASSERT(b.len() == 3);
		STF_ASSERT(b.cap() == 3);
		STF_ASSERT(b[0] == 3 && b[1] == 4 && b[2] == 5);
	}

	{
		// initializer list operator=
		zbs::vector<int> a = {3, 4};
		a = {10};
		STF_ASSERT(a.len() == 1);
		STF_ASSERT(a.cap() == 2);
		STF_ASSERT(a[0] == 10);
		a = {7, 8, 9};
		STF_ASSERT(a.len() == 3);
		STF_ASSERT(a.cap() == 3);
		STF_ASSERT(a[0] == 7 && a[1] == 8 && a[2] == 9);
	}

	{
		// copy operator=
		zbs::vector<int> a = {3, 4};
		zbs::vector<int> b;
		b = a;
		STF_ASSERT(b.len() == 2);
		STF_ASSERT(b.cap() == 2);
		STF_ASSERT(b[0] == 3 && b[1] == 4);
	}

	{
		// move operator=
		zbs::vector<int> a = {3, 4};
		zbs::vector<int> b;
		b = std::move(a);
		STF_ASSERT(b.len() == 2);
		STF_ASSERT(b.cap() == 2);
		STF_ASSERT(b[0] == 3 && b[1] == 4);
		STF_ASSERT(a.len() == 0);
		STF_ASSERT(a.cap() == 0);
		STF_ASSERT(a.data() == nullptr);
	}
}

bool check_vector(zbs::vector<int> &v, const zbs::vector<int> &gold) {
	if (v.len() != gold.len()) {
		return false;
	}

	for (int i = 0; i < v.len(); i++) {
		if (v[i] != gold[i]) {
			return false;
		}
	}
	return true;
}

STF_TEST("vector methods") {
	{
		// clear
		zbs::vector<int> a = {1, 2, 3};
		a.clear();
		STF_ASSERT(a.len() == 0);
		STF_ASSERT(a.cap() == 3);
		STF_ASSERT(a.data() != nullptr);
	}

	{
		// reserve
		zbs::vector<int> a = {0, 5, 10};
		a.reserve(10);
		STF_ASSERT(a.cap() >= 10);
		STF_ASSERT(check_vector(a, {0, 5, 10}));
	}
	{
		// shrink
		zbs::vector<int> a = {0, 5, 10};
		a.reserve(10);
		a.shrink();
		STF_ASSERT(a.cap() == 3);
		STF_ASSERT(check_vector(a, {0, 5, 10}));
	}
	{
		// resize
		zbs::vector<int> a;
		a.resize(3);
		STF_ASSERT(a.cap() >= 3);
		STF_ASSERT(check_vector(a, {0, 0, 0}));

		zbs::vector<int> b = {-1, -2, -3};
		b.resize(1);
		STF_ASSERT(b.cap() == 3);
		STF_ASSERT(check_vector(b, {-1}));

		zbs::vector<int> c = {1, 2};
		c.resize(5, 3);
		STF_ASSERT(c.cap() >= 5);
		STF_ASSERT(check_vector(c, {1, 2, 3, 3, 3}));
	}

	{
		// insert_after
		zbs::vector<int> a = {1, 2, 3};
		a.insert_after(0, 100);
		STF_ASSERT(check_vector(a, {1, 100, 2, 3}));

		zbs::vector<int> b = {5, 6};
		b.insert_after(1, 10);
		STF_ASSERT(check_vector(b, {5, 6, 10}));

		zbs::vector<int> c = {-1, -2, -3, -4, -5};
		c.insert_after(2, 0);
		STF_ASSERT(check_vector(c, {-1, -2, -3, 0, -4, -5}));

		zbs::vector<int> d = {10, -10};
		zbs::vector<int> e = {1, 2, 3};
		e.insert_after(2, d.sub());
		STF_ASSERT(check_vector(e, {1, 2, 3, 10, -10}));
		e.insert_after(0, d.sub());
		STF_ASSERT(check_vector(e, {1, 10, -10, 2, 3, 10, -10}));
		e.insert_after(1, d.sub());
		STF_ASSERT(check_vector(e, {1, 10, 10, -10, -10, 2, 3, 10, -10}));
	}

	{
		// insert_before
		zbs::vector<int> a = {1, 2, 3};
		a.insert_before(0, 0);
		STF_ASSERT(check_vector(a, {0, 1, 2, 3}));

		zbs::vector<int> b = {1, 2, 3};
		b.insert_before(2, 333);
		STF_ASSERT(check_vector(b, {1, 2, 333, 3}));

		zbs::vector<int> c = {10, -10};
		zbs::vector<int> d = {1, 2, 3};
		d.insert_before(0, c.sub());
		STF_ASSERT(check_vector(d, {10, -10, 1, 2, 3}));
		d.insert_before(4, c.sub());
		STF_ASSERT(check_vector(d, {10, -10, 1, 2, 10, -10, 3}));
		d.insert_before(2, c.sub());
		STF_ASSERT(check_vector(d, {10, -10, 10, -10, 1, 2, 10, -10, 3}));
	}

	{
		// append
		zbs::vector<int> a = {};
		for (int i = 0; i < 10; i++) {
			a.append(i);
		}
		STF_ASSERT(a.cap() >= 10);
		STF_ASSERT(check_vector(a, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));

		zbs::vector<int> b = {1, 2};
		zbs::vector<int> c = {10, 20, 30, 40, 50};
		b.append(c.sub(1, 3));
		STF_ASSERT(check_vector(b, {1, 2, 20, 30}));
	}

	{
		// remove
		zbs::vector<int> a = {1, 2, 3};
		a.remove(0);
		STF_ASSERT(check_vector(a, {2, 3}));
		a.remove(1);
		STF_ASSERT(check_vector(a, {2}));

		zbs::vector<int> b = {1, 2, 3, 4, 5, 6};
		b.remove(4, 6);
		STF_ASSERT(check_vector(b, {1, 2, 3, 4}));
		b.remove(1, 3);
		STF_ASSERT(check_vector(b, {1, 4}));
		b.remove(0, 2);
		STF_ASSERT(check_vector(b, {}));
	}
}
