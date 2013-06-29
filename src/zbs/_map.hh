#pragma once

#include <cstdlib>
#include <type_traits>
#include <initializer_list>
#include <utility>
#include <new>
#include "_types.hh"
#include "_utils.hh"

namespace zbs {

// Hashmap.
//
// K - type of the key
// V - type of the value
// Hash - takes (const K&, int) and returns int, a hash functor with seed
// KeyEqual - checks if one key is equal to another, returns bool

template <typename T>
struct hash;

template <>
struct hash<const char*> {
	int operator()(const char *s, int seed);
};

// TODO: more hash implementations

template <typename K, typename V>
struct key_and_value {
	K key;
	V value;
};

template <typename K, typename V, typename Hash>
class map_iter;

template <typename K, typename V, typename Hash = hash<K>>
class map {
	friend class map_iter<K, V, Hash>;

	static constexpr float _load = 6.5;
	static constexpr int _bucket_size = 8;
	static constexpr int _max_key_size = 128;
	static constexpr int _max_value_size = 128;
	static constexpr bool _indirect_key() {	return sizeof(K) > _max_key_size; }
	static constexpr bool _indirect_value() { return sizeof(V) > _max_value_size; }

	typedef typename std::conditional<_indirect_key(), K*, K>::type KK;
	typedef typename std::conditional<_indirect_value(), V*, V>::type VV;

	template <bool Indirect, typename T>
	struct _indirect;

	// when T is indirectly stored, all functions take T*
	template <typename T>
	struct _indirect<true, T> {
		static T &get(T *a) {
			return *a;
		}

		static void destroy(T *a) {
			a->~T();
			detail::free(a);
		}

		static T *insert(T *a) {
			*a = detail::malloc<T>(1);
			return *a;
		}
	};

	// when T is directly stored, all functions take T&
	template <typename T>
	struct _indirect<false, T> {
		static T &get(T &a) {
			return a;
		}

		static void destroy(T &a) {
			a.~T();
		}

		static T *insert(T &a) {
			return &a;
		}
	};

	struct _bucket {
		uint8 top_hash[_bucket_size];
		_bucket *overflow;
		KK keys[_bucket_size];
		VV values[_bucket_size];

		K &key(int i) {
			return _indirect<_indirect_key(), K>::get(keys[i]);
		}

		V &value(int i) {
			return _indirect<_indirect_value(), V>::get(values[i]);
		}

		void clear() {
			static_assert(_bucket_size == 8, "bucket_size != 8");
			*(uint64*)top_hash = 0;
			overflow = nullptr;
		}

		void free() {
			for (int i = 0; i < _bucket_size; i++) {
				if (top_hash[i] == 0)
					continue;

				_indirect<_indirect_key(), K>::destroy(keys[i]);
				_indirect<_indirect_value(), V>::destroy(values[i]);
			}
		}
	};

	int _hash0 = 0;
	int _count = 0;
	uint8 _B = 0;
	_bucket *_buckets = nullptr;

	void _split_bucket(_bucket *b, int i) {
		int newbit = 1 << (_B - 1);
		_bucket *x = _buckets + i;
		_bucket *y = _buckets + i + newbit;
		x->clear();
		y->clear();
		int xi = 0;
		int yi = 0;

		do {
			for (int i = 0; i < _bucket_size; i++) {
				if (b->top_hash[i] == 0)
					continue;

				int hash = Hash()(b->key(i), _hash0);
				if ((hash & newbit) == 0) {
					if (xi == _bucket_size) {
						_bucket *newx = detail::malloc<_bucket>(1);
						newx->clear();
						x->overflow = newx;
						x = newx;
						xi = 0;
					}
					x->top_hash[xi] = b->top_hash[i];
					new (&x->keys[xi]) KK(std::move(b->keys[i]));
					new (&x->values[xi]) VV(std::move(b->values[i]));
					xi++;
				} else {
					if (yi == _bucket_size) {
						_bucket *newy = detail::malloc<_bucket>(1);
						newy->clear();
						y->overflow = newy;
						y = newy;
						yi = 0;
					}
					y->top_hash[yi] = b->top_hash[i];
					new (&y->keys[yi]) KK(std::move(b->keys[i]));
					new (&y->values[yi]) VV(std::move(b->values[i]));
					yi++;
				}
			}
			b = b->overflow;
		} while (b);
	}

	void _grow() {
		_bucket *old_buckets = _buckets;
		int old_buckets_n = 1 << _B;

		_B++;
		_buckets = detail::malloc<_bucket>(1 << _B);

		for (int i = 0; i < old_buckets_n; i++) {
			_bucket *b = old_buckets + i;
			_split_bucket(b, i);

			// free old bucket contents and overflow buckets
			_bucket *next = b->overflow;
			while (next) {
				_bucket *cur = next;
				next = cur->overflow;

				cur->free();
				detail::free(cur);
			}
			b->free();
		}
		detail::free(old_buckets);
	}

	V &_find_or_insert(K key) {
		int hash = Hash()(key, _hash0);
		if (_buckets == nullptr) {
			_buckets = detail::malloc<_bucket>(1);
			_buckets->clear();
		}

again:
		int bi = hash & ((1 << _B) - 1);
		_bucket *b = _buckets + bi;
		uint8 top = hash >> (sizeof(int) * 8 - 8);
		if (top == 0)
			top = 1;

		uint8 *insert_top = nullptr;
		KK *insert_key = nullptr;
		VV *insert_value = nullptr;

		for (;;) {
			for (int i = 0; i < _bucket_size; i++) {
				if (b->top_hash[i] != top) {
					if (b->top_hash[i] == 0 && insert_top == nullptr) {
						insert_top = b->top_hash + i;
						insert_key = b->keys + i;
						insert_value = b->values + i;
					}
					continue;
				}

				if (!(key == b->key(i)))
					continue;

				return b->value(i);
			}

			if (b->overflow == nullptr)
				break;
			b = b->overflow;
		}

		if (_count >= _load * (1 << _B) && _count >= _bucket_size) {
			_grow();
			goto again;
		}

		if (insert_top == nullptr) {
			_bucket *newb = detail::malloc<_bucket>(1);
			newb->clear();
			b->overflow = newb;
			insert_top = newb->top_hash;
			insert_key = &newb->key(0);
			insert_value = &newb->value(0);
		}

		*insert_top = top;
		K *newk = _indirect<_indirect_key(), K>::insert(*insert_key);
		V *newv = _indirect<_indirect_value(), V>::insert(*insert_value);
		new (newk) K(std::move(key));
		new (newv) V;
		_count++;

		return *newv;
	}

public:
	explicit map(int hint) {
		while (hint > _bucket_size && hint > _load * (1 << _B))
			_B++;

		if (_B != 0) {
			_buckets = detail::malloc<_bucket>(1 << _B);
			for (int i = 0, n = 1 << _B; i < n; i++) {
				_buckets[i].clear();
			}
		}
		_hash0 = detail::fastrand();
	}

	map(): map(0) {}

	map(std::initializer_list<key_and_value<K, V>> r): map(r.size()) {
		for (const auto &kv : r) {
			operator[](kv.key) = kv.value;
		}
	}

	~map() {
		if (_buckets == nullptr)
			return;

		clear();
		detail::free(_buckets);
	}

	void clear() {
		if (_buckets == nullptr)
			return;

		for (int i = 0, n = 1 << _B; i < n; i++) {
			_bucket &b = _buckets[i];
			_bucket *next = b.overflow;
			while (next) {
				_bucket *cur = next;
				next = cur->overflow;

				cur->free();
				detail::free(cur);
			}
			b.free();
		}
		_count = 0;
	}

	int len() const { return _count; }
	int cap() const { return _load * (1 << _B); }

	V &operator[](K k) {
		return _find_or_insert(std::move(k));
	}
};

template <typename K, typename V, typename Hash>
class map_iter {
	typename map<K, V, Hash>::_bucket *_buckets;
	typename map<K, V, Hash>::_bucket *_bucket;
	int _buckets_n;
	int _bucket_i;
	int _i;

	void _find_next_valid() {
		if (_bucket_i == _buckets_n) {
			return;
		}

		for (;;) {
			_i++;
			if (_i == map<K, V, Hash>::_bucket_size) {
				// done with current bucket, try next overflow
				_bucket = _bucket->overflow;
				_i = 0;
				if (_bucket == nullptr) {
					// oops, no overflow bucket, try next
					// one in the table
					_bucket_i++;
					if (_bucket_i == _buckets_n) {
						// no buckets left, we're done
						return;
					}
					_bucket = _buckets + _bucket_i;
				}
			}

			// check if it's a valid entry
			if (_bucket->top_hash[_i] != 0) {
				return;
			}
		}
	}
public:
	map_iter() = default;

	explicit map_iter(map<K, V, Hash> &m):
		_buckets(m._buckets), _bucket(m._buckets),
		_buckets_n(1 << m._B), _bucket_i(0), _i(0)
	{
		if (m.len() == 0) {
			_buckets_n = 0;
			return;
		}

		if (_bucket->top_hash[_i] == 0)
			_find_next_valid();
	}

	map_iter &operator++() {
		_find_next_valid();
		return *this;
	}

	bool operator==(const map_iter&) const { return _bucket_i == _buckets_n; }
	bool operator!=(const map_iter&) const { return _bucket_i != _buckets_n; }
	key_and_value<const K&, V&> operator*() {
		return {
			map<K, V, Hash>::template _indirect<
				map<K, V, Hash>::_indirect_key(), K
			>::get(_bucket->keys[_i]),
			map<K, V, Hash>::template _indirect<
				map<K, V, Hash>::_indirect_value(), V
			>::get(_bucket->values[_i]),
		};
	}
};

template <typename K, typename V, typename Hash>
map_iter<K, V, Hash> begin(map<K, V, Hash> &m) {
	return map_iter<K, V, Hash>(m);
}

template <typename K, typename V, typename Hash>
map_iter<K, V, Hash> end(map<K, V, Hash>&) {
	return map_iter<K, V, Hash>();
}

} // namespace zbs
