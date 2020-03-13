#pragma once
#include <vector>
#include "hash_primitives.h"
#include "optional.h"

template <typename K, typename V>
class Entry {
	K k;
	K &key;
	V v;
	V &value;

public:
	Entry(K &key, V &value) : k(key), key(key), v(value), value(value) {}

	K &getKey() {
		if (key != k)
			return k;
		return key;
	}
	
	V &getValue() {
		if (value != v)
			return v;
		return value;
	}

};

template <typename K, typename V>
class HashMap {

	bool owner;
	size_t length, count;
	double loadFactor = 0.0;
	const double rehashFactor = 0.65;
	Optional<K> *keys;
	V *values;

	size_t (*hash)(K key);

	size_t hashKey(K key) {
		if (hash != nullptr)
			return hash(key) % length;
		else if (primitive<K>::exists()) {
			size_t x = primitive<K>::hash(key);
			x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
			x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
			x = x ^ (x >> 31);
			return x % length;
		} else if (std::string *val = reinterpret_cast<std::string *>(&key)) {
			size_t hash = 7;
			for (size_t i = 0; i < val->size(); i++)
				hash = (hash * 17) + ((*val)[i] * i + 1);
			return hash % length;
		}
		throw "Unable to Hash Key.";
	}

	void rehash() {
		Optional<K> *keysCopy = keys;
		V *valuesCopy = values;
		loadFactor = 0.0;
		count = 0;
		length *= 2;
		keys = new Optional<K>[length];
		values = new V[length];
		for (size_t i = 0; i < length / 2; i++) {
			if (keysCopy[i].exists) {
				put(keysCopy[i].value, valuesCopy[i]);
			}
		}
		delete[] keysCopy;
		delete[] valuesCopy;
	}

public:
	HashMap() : owner(true), length(2), count(0), keys(new Optional<K>[length]), values(new V[length]), hash(NULL) {}
	HashMap(size_t length, size_t (*hash)(K key)) : owner(true), length(length), count(0), keys(new Optional<K>[length], values(new V[length]), hash(hash)) {}
	HashMap(const HashMap<K, V> &other) : owner(false), length(other.length), count(other.count), keys(other.keys), values(other.values), hash(other.hash) {}
	~HashMap() {
		if (owner) {
			delete[] keys;
			delete[] values;
		}
	}

	HashMap<K, V> &operator=(const HashMap<K, V> &other) {
		HashMap<K, V> temp = HashMap<K, V>(other);
		std::swap(this->owner, temp.owner);
		std::swap(this->length, temp.length);
		std::swap(this->count, temp.count);
		std::swap(this->loadFactor, temp.loadFactor);
		std::swap(this->keys, temp.keys);
		std::swap(this->values, temp.values);
		std::swap(this->hash, temp.hash);
		return *this;
	}

	HashMap<K, V> &put(K key, V value) {
		if (loadFactor >= rehashFactor)
			rehash();
		size_t index = hashKey(key);
		Optional<K> &actualKey = this->keys[index];
		if (actualKey.exists) {
			rehash();
			return put(key, value);
		}
		actualKey.exists = true;
		actualKey.value = key;
		values[index] = value;
		count++;
		loadFactor = double(count) / double(length);
		return *this;
	}

	Optional<V *> get(K key) {
		size_t index = hashKey(key);
		Optional<K> &actualKey = keys[index];
		if (actualKey.exists)
			return Optional<V *>(&values[index]);
		return Optional<V *>();
	}

	V &getOrPutIfEmpty(K key, V value) {
		size_t index = hashKey(key);
		Optional<K> &actualKey = keys[index];
		if (!actualKey.exists || actualKey.value != key)
			put(key, value);
		return values[index];
	}

	void getEntries(std::vector<Entry<K, V>> &entries) {
		for (size_t i = 0; i < length; i++) {
			if (keys[i].exists) {
				entries.push_back(Entry<K, V>(keys[i].value, values[i]));
			}
		}
	}

	size_t size() { return count; }

};
