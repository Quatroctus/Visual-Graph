#pragma once
#include "hash_primitives.h"
#include "optional.h"
#include <string>
#include <vector>

template <typename K, typename V>
struct Map {
	virtual ~Map() {}

	virtual Map<K, V> &put(K key, V val) = 0;

	virtual Optional<V> remove(K key) = 0;

	virtual bool contains(K key) = 0;

	virtual Optional<V> query(K key) = 0;

	virtual V &queryOrPutIfEmpty(K key, V val) = 0;

	virtual void getEntries(std::vector<std::pair<K, V>> *entries, bool empty=false) = 0;

	virtual size_t size() = 0;

	struct Entry {
		bool isValue;
		union Something {
			V value;
			Map<K, V> *map;
			Something() : value(V()) {}
			~Something() {
				value.~V();
			}
		} entry;
		Entry() : isValue(true) {}
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
		std::vector<std::pair<K, V>> entries;
		getEntries(&entries);
		length *= 2;
		count = 0;
		loadFactor = 0.0F;
		createArrays();
		for (std::pair<K, V> entry : entries) {
			this->put(entry.first, entry.second);
		}
	}

public:
	HashMap() : length(2), count(0), depth(0), loadFactor(0.0F), hash(nullptr) {
		createArrays();
	}
	HashMap(size_t length, size_t depth, size_t (*hash)(K key)) : length(length), count(0), depth(depth), loadFactor(0.0F), hash(hash) {
		createArrays();
	}
	HashMap(size_t (*hash)(K key)) : length(2), count(0), depth(0), loadFactor(0.0F), hash(hash) {
		createArrays();
	}
	HashMap(size_t length, size_t (*hash)(K key) = nullptr) : length(length), count(0), depth(0), loadFactor(0.0F), hash(hash) {
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
		} else if (values[index].isValue) {
			Map<K, V>::template Entry &entry = values[index];
			V val = entry.entry.value;
			entry.isValue = false;
			entry.entry.map = new HashMap<K, V>(2, depth + 1, hash);
			entry.entry.map->put(actualKey.value, val);
			entry.entry.map->put(key, value);
		} else {
			values[index].entry.map->put(key, value);
		}
		count++;
		loadFactor = float(count / length);
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
			} else {
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
		size_t index = hashEntry(key);
		Optional<K> &actualKey = keys[index];
		if (actualKey.exists) {
			Map<K, V>::template Entry &entry = values[index];
			if (entry.isValue) {
				if (actualKey.value == key) {
					return entry.entry.value;
				} else {
					V val = entry.entry.value;
					entry.isValue = false;
					entry.entry.map = new HashMap<K, V>(2, depth + 1, hash);
					entry.entry.map->put(actualKey.value, val);
					entry.entry.map->put(key, value);
					return entry.entry.map->queryOrPutIfEmpty(key, value);
				}
			}
			else
				return entry.entry.map->queryOrPutIfEmpty(key, value);
		}
		else {
			values[index].entry.value = value;
			count++;
			loadFactor = (float) count / (float) length;
			actualKey.exists = true;
			return values[index].entry.value;
		}
	}

	void getEntries(std::vector<std::pair<K, V>> *entries, bool empty = false) override {
		for (size_t i = 0; i < length; i++) {
			Optional<K> &actualKey = keys[i];
			if (actualKey.exists) {
				Map<K, V>::template Entry &value = values[i];
				if (value.isValue && entries != NULL)
					entries->push_back(std::make_pair(actualKey.value, value.entry.value));
				else if (!value.isValue) {
					value.entry.map->getEntries(entries, empty);
					if (empty)
						delete value.entry.map;
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
