#pragma once
#include "hash_primitives.h"
#include "optional.h"
#include <string>
#include <vector>

template <typename K, typename V>
class KeyValue {
	K k;
	K &key;
	V v;
	V &value;

public:
	KeyValue(K &key, V &value) : k(key), key(key), v(value), value(value) {}

	K &getKey() {
		if (key != k) {
			return k;
		}
		return key;
	}

	V &getValue() {
		if (value != v) {
			return v;
		}
		return value;
	}

};

template <typename K, typename V>
struct Map {
	virtual ~Map() {}

	virtual Map<K, V> &put(K key, V val) = 0;

	virtual Optional<V> remove(K key) = 0;

	virtual bool contains(K key) = 0;

	virtual Optional<V> query(K key) = 0;

	virtual V &queryOrPutIfEmpty(K key, V val) = 0;

	virtual void getEntries(std::vector<KeyValue<K, V>> *entries, bool empty = false) = 0;

	virtual size_t size() = 0;

	struct Entry {
		bool isValue;
		union Something {
			V value;
			Map<K, V> *map;
			Something() : value(V()) {}
			Something(Something &something) : value(something.value) {}
			~Something() {
				value.~V();
			}
		} entry;
		Entry() : isValue(true) {}
		Entry(Entry &entry) : isValue(entry.isValue), entry(entry.entry) {}
	};
};

template <typename K, typename V>
class HashMap : public Map<K, V> {
	size_t length, count, depth;
	float loadFactor;
	const float reHashFactor = 0.65F;
	Map<K, V>::Entry *values;
	Optional<K> *keys;

	size_t (*hash)(K key);

	void createArrays() {
		values = new Map<K, V>::template Entry[length];
		keys = new Optional<K>[length];
	}

	size_t hashEntry(K key) {
		size_t depthMultiplier = 13 * depth;
		for (size_t i = 0; i < depth; i++)
			depthMultiplier *= 29;
		if (hash != nullptr)
			return (hash(key) * depthMultiplier) % length;
		else if (primitive<K>::exists())
			size_t x = primitive<K>::hash(key);
			x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
			x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
			x = x ^ (x >> 31);
			return x;
		else if (std::string *val = reinterpret_cast<std::string *>(&key)) {
			size_t hash = 7;
			for (size_t i = 0; i < val->size(); i++)
				hash = (hash * 17) + ((*val)[i] * i + 1);
			return (hash * depthMultiplier) % length;
		}
		throw "Unable to Hash Key.";
	}

	void rehash() {
		std::vector<KeyValue<K, V>> entries;
		getEntries(&entries, true);
		length *= 2;
		count = 0;
		loadFactor = 0.0F;
		createArrays();
		for (KeyValue<K, V> entry : entries) {
			this->put(entry.getKey(), entry.getValue());
		}
	}

public:
	HashMap() : length(4), count(0), depth(1), loadFactor(0.0F), hash(nullptr) {
		createArrays();
	}
	HashMap(size_t length, size_t depth, size_t (*hash)(K key)) : length(length), count(0), depth(depth), loadFactor(0.0F), hash(hash) {
		createArrays();
	}
	HashMap(size_t (*hash)(K key)) : length(4), count(0), depth(1), loadFactor(0.0F), hash(hash) {
		createArrays();
	}
	HashMap(size_t length, size_t (*hash)(K key) = nullptr) : length(length), count(0), depth(1), loadFactor(0.0F), hash(hash) {
		createArrays();
	}
	~HashMap() override {
		if (values != NULL)
			getEntries(NULL);
	}

	Map<K, V> &put(K key, V value) override {
		if (loadFactor >= reHashFactor)
			rehash();
		size_t index = hashEntry(key);
		Optional<K> &actualKey = this->keys[index];
		if (!actualKey.exists || actualKey.value == key) {
			actualKey.exists = true;
			actualKey.value = key;
			if (values[index].isValue)
				values[index].entry.value = value;
			else
				values[index].entry.map->put(key, value);
		}
		else if (values[index].isValue) {
			Map<K, V>::template Entry &entry = values[index];
			V val = entry.entry.value;
			entry.isValue = false;
			entry.entry.map = new HashMap<K, V>(2, depth + 1, hash);
			entry.entry.map->put(actualKey.value, val);
			entry.entry.map->put(key, value);
		}
		else {
			values[index].entry.map->put(key, value);
		}
		count++;
		loadFactor = float(count) / float(length);
		return *this;
	}

	Optional<V> remove(K key) override {
		size_t index = hashEntry(key);
		Optional<K> &actualKey = this->keys[index];
		if (actualKey.exists) {
			if (values[index].isValue && actualKey.value == key) {
				actualKey.exists = false;
				count--;
				loadFactor = float(count / length);
				return values[index].entry.value;
			}
			else {
				Optional<V> optVal = values[index].entry.map->remove(key);
				if (optVal.exists) {
					count--;
					loadFactor = float(count / length);
				}
				return optVal;
			}
		}
		return Optional<V>();
	}

	bool contains(K key) override {
		size_t index = hashEntry(key);
		Optional<K> &actualKey = keys[index];
		Map<K, V>::template Entry &value = values[index];
		return actualKey.exists && ((value.isValue && actualKey.value == key) || (!value.isValue && value.entry.map->contains(key)));
	}

	Optional<V> query(K key) override {
		size_t index = hashEntry(key);
		Optional<K> &actualKey = keys[index];
		if (!actualKey.exists)
			return Optional<V>();
		Map<K, V>::template Entry &value = values[index];
		if (!value.isValue)
			return value.entry.map->query(key);
		if (actualKey.value == key)
			return Optional<V>(value.entry.value);
		return Optional<V>();
	}

	V &queryOrPutIfEmpty(K key, V value) {
		if (!contains(key))
			put(key, value);
		size_t index = hashEntry(key);
		if (values[index].isValue) {
			return values[index].entry.value;
		}
		return values[index].entry.map->queryOrPutIfEmpty(key, value);
	}

	void getEntries(std::vector<KeyValue<K, V>> *entries, bool empty = false) override {
		for (size_t i = 0; i < length; i++) {
			Optional<K> &actualKey = keys[i];
			if (actualKey.exists) {
				if (empty) {
					Map<K, V>::template Entry value = values[i];
					if (value.isValue && entries != NULL)
						entries->push_back(KeyValue<K, V>(actualKey.value, value.entry.value));
					else if (!value.isValue) {
						value.entry.map->getEntries(entries, empty);
						delete value.entry.map;
					}
				}
				else {
					Map<K, V>::template Entry &value = values[i];
					if (value.isValue && entries != NULL)
						entries->push_back(KeyValue<K, V>(actualKey.value, value.entry.value));
					else if (!value.isValue) {
						value.entry.map->getEntries(entries, empty);
					}
				}
			}
		}
		if (empty) {
			delete[] values;
			delete[] keys;
			values = NULL;
			keys = NULL;
		}
	}

	size_t size() { return count; }
};
