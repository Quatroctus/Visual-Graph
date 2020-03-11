#pragma once
#include <vector>
#include "hash_map.h"

template <typename V>
struct Node {
	SDL_Rect pos;
	V value;
	Node() : pos({0, 0, 24, 16}), value(V()) {}
	Node(int x, int y, V value) : pos({ x, y, 24, 16 }), value(value) {}
};

template <typename V>
struct Arc {
	int weight;
	V connectedValue;
	Arc() : weight(0), connectedValue(V()) {}
	Arc(int weight, V value) : weight(weight), connectedValue(value) {}
};

template <typename V>
bool operator!=(const std::pair<Node<V>, std::vector<Arc<V>>> &first, std::pair<Node<V>, std::vector<Arc<V>>> second) {
	return first.first.value != second.first.value;
}

template <typename V>
class Graph;

template <typename V>
struct Connection {
	V value;
	Graph<V> *owner;
	HashMap<V, int> connections;
	Connection(V value, Graph<V> *owner) : value(value), owner(owner), connections(HashMap<V, int>()) {}

	Connection<V> &addArc(V value, int weight) {
		this->connections.put(value, weight);
		return *this;
	}

	Graph<V> &finish() {
		this->owner->finish(*this);
		return *owner;
	}

};

template <typename V>
class Graph {

	HashMap<V, std::pair<Node<V>, std::vector<Arc<V>>>> nodeMap;

	Node<V> createNode(V value) {
		int n = (int) nodeMap.size() + 1;
		int k = (int) ceil((sqrt(n) - 1.0) / 2.0);
		int t = 2 * k + 1;
		int m = t * t;
		t -= 1;
		if (n >= (m - t))
			return Node<V>((k - (m - n)) * 48, (-k) * 32, value);
		m -= t;
		if (n >= (m - t))
			return Node<V>((-k) * 48, (-k + (m - n)) * 32, value);
		m -= t;
		if (n >= (m - t))
			return Node<V>((-k + (m - n)) * 48, (k)*32, value);
		return Node<V>((k)*48, (k - (m - n - t)) * 32, value);
	}

public:

	Graph() : nodeMap(HashMap<V, std::pair<Node<V>, std::vector<Arc<V>>>>()) {}

	Connection<V> addValue(V value) {
		return Connection<V>(value, this);
	}

	Graph<V> &addArc(V first, V second, int weight) {
		std::pair<Node<V>, std::vector<Arc<V>>> &firstEntry = nodeMap.queryOrPutIfEmpty(first, std::make_pair(createNode(first), std::vector<Arc<V>>()));
		bool found = false;
		for (Arc<V> arc : firstEntry.second) {
			if (arc.connectedValue == second) {
				arc.weight = (int) fmin(arc.weight, weight);
				found = true;
			}
		}
		if (!found)
			firstEntry.second.push_back(Arc<V>(weight, second));
		std::pair<Node<V>, std::vector<Arc<V>>> &secondEntry = nodeMap.queryOrPutIfEmpty(second, std::make_pair(createNode(second), std::vector<Arc<V>>()));
		found = false;
		for (Arc<V> arc : secondEntry.second) {
			if (arc.connectedValue == first) {
				arc.weight = (int) fmin(arc.weight, weight);
				found = true;
			}
		}
		if (!found)
			secondEntry.second.push_back(Arc<V>(weight, first));
		return *this;
	}

	Graph<V> &finish(Connection<V> &connection) {
		std::vector<KeyValue<V, int>> arcs;
		connection.connections.getEntries(&arcs);
		nodeMap.queryOrPutIfEmpty(connection.value, std::make_pair(createNode(connection.value), std::vector<Arc<V>>()));
		for (KeyValue<V, int> arcData : arcs) {
			addArc(connection.value, arcData.getKey(), arcData.getValue());
		}
		return *this;
	}

	HashMap<V, std::pair<Node<V>, std::vector<Arc<V>>>> getNodeMap() {
		return nodeMap;
	}

};
