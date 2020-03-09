#include <math.h>
#include "graph.h"

template <typename V>
Node<V>::Node() : x(0), y(0), value(V()) {}

template <typename V>
Node<V>::Node(int x, int y, V value) : x(x), y(y), value(value) {}

template <typename V>
Arc<V>::Arc() : weight(0), connectedValue(V()) {}

template <typename V>
Arc<V>::Arc(int weight, V value) : weight(weight), connectedValue(value) {}

template <typename V>
Connection<V>::Connection(V value, Graph<V> *owner) : value(value), owner(owner), connections(HashMap<V, int>()) {}

template <typename V>
Connection<V> &Connection<V>::addArc(V value, int weight) {
	this->connections.insert(value, weight);
}

template <typename V>
Graph<V> &Connection<V>::finish() {
	this->owner->finish(*this);
}

template <typename V>
Graph<V>::Graph() : nodes(HashMap<V, std::pair<Node<V>, std::vector<Arc<V>>>>()) {}

template <typename V>
Node<V> Graph<V>::createNode(V value) {
	size_t n = nodes.size() + 1;
	int k = ceil((sqrt(n) - 1.0) / 2.0);
	int t = 2 * k + 1;
	int m = t * t;
	t -= 1;
	if (n >= m - t)
		return Node<V>(value, (k - (m - n)) * 48, (-k) * 32);
	m -= t;
	if (n >= m - t)
		return Node<V>(value, (-k) * 48, (-k + (m - n)) * 32);
	m -= t;
	if (n >= m - t)
		return Node<V>(value, (-k + (m - n)) * 48, (k) * 32);
	return Node<V>(value, (k) * 48, (k - (m - n)) * 32);
}

template <typename V>
Connection<V> Graph<V>::addValue(V value) {
	return Connection<V>(value, this);
}

template <typename V>
Graph<V> &Graph<V>::addArc(V first, V second, int weight) {
	std::pair<Node<V>, std::vector<Arc<V>>> &firstEntry = nodes.queryOrPutIfEmpty(first, std::make_pair(createNode(first), std::vector<Arc<V>>()));
	std::pair<Node<V>, std::vector<Arc<V>>> &secondEntry = nodes.queryOrPutIfEmpty(second, std::make_pair(createNode(second), std::vector<Arc<V>>()));
	bool found = false;
	for (Arc<V> arc : firstEntry.second) {
		if (arc.connectedValue == second) {
			arc.weight = fmin(arc.weight, weight);
			found = true;
		}
	}
	if (!found)
		firstEntry.second.push_back(Arc<V>(second, weight));
	found = false;
	for (Arc<V> arc : secondEntry.second) {
		if (arc.connectedValue == first){
			arc.weight = fmin(arc.weight, weight);
			found = true;
		}
	}
	if (!found)
		secondEntry.second.push_back(Arc<V>(first, weight));
	return *this;
}

template <typename V>
Graph<V> &Graph<V>::finish(Connection<V> &connection) {
	std::vector<std::pair<V, int>> arcs;
	connection.connections.getEntries(&arcs);
	for (std::pair<V, int> arcData : arcs) {
		addArc(connection.value, arcData.first, arcData.second);
	}
	return *this;
}
