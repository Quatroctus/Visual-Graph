#pragma once
#include <vector>
#include "optional.h"

// TODO
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

	size_t length, count, depth;
	double loadFactor = 0.0D;
	const double rehashFactor = 0.65;
	Optional<K> *keys;
	V *values;

	size_t (*hash)(K key);

	size_t hashKey(K key) {
		size_t depthMultiplier = 13 * depth;
		for (size_t i = 0; i < depth; i++)
			depthMultiplier *= 29;
		if (hash != nullptr)
			return (hash(key) * depthMultiplier) % length;
		else if (primitive<K>::exists())
			return (primitive<K>::hash(key) * 31 * depthMultiplier) % length;
		else if (std::string *val = reinterpret_cast<std::string *>(&key)) {
			size_t hash = 7;
			for (size_t i = 0; i < val->size(); i++)
				hash = (hash * 17) + ((*val)[i] * i + 1);
			return (hash * depthMultiplier) % length;
		}
		throw "Unable to Hash Key.";
	}

	void rehash() {

	}

public:
	HashMap() : length(2), count(0), depth(1), keys(new Optional<K>[length], values(new Entry<K, V>[length]), hash(NULL)) {}
	HashMap(size_t length, size_t depth, size_t (*hash)(K key)) : length(length), count(0), depth(depth), keys(new Optional<K>[length]), values(new Entry<K, V>[length]), hash(hash)) {}

	HashMap<K, V> &put(K key, V &value) override {
		if (loadFactor >= rehashFactor)
			rehash();
		size_t index = hashKey(key);
		Optional<K> &actualKey = this->keys[index];
		if (!actualKey.exists || actualKey.value == key) {
			actualKey.exists = true;
			actualKey.value = key;
			if (values[index].isValue)
				values[index].value.value = value;
			else
				values[index].value.map->put(key, value);
		}
		else if (values[index].isValue) {
			Entry<K, V> &entry = values[index];
			V *val = entry.value.value;
			entry.isValue = false;
			entry.value.map = new HashMap<K, V>(2, depth + 1, hash);
			entry.value.map->put(actualKey.value, *val);
			entry.value.map->put(key, value);
		}
		else {
			values[index].value.map->put(key, value);
		}
		count++;
		loadFactor = float(count / length);
		return *this;
	}

	V &get(K key) override {
		size_t index = hashEntry(key);
		Optional<K> &actualKey = keys[index];
		if (!actualKey.exists)
			return Optional<V>();
		Entry<K, V> &value = values[index];
		if (!value.isValue)
			return value.entry.map->get(key);
		if (actualKey.value == key)
			return Optional<V>(value.entry.value);
		return Optional<V>();
	}

	V &getOrPutIfEmpty(K key, Value value) {
	}
};
