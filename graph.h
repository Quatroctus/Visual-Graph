#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "hashmap.h"

template <typename V>
struct Node {

	SDL_Rect pos;
	V value;
	int *refCount;
	SDL_Texture *texture;
	bool selected;

	Node() : pos({ 0, 0, 24, 16 }), value(V()), texture(NULL), selected(false) {
		refCount = new int;
		refCount[0] = 1;
	}
	Node(int x, int y, V value) : pos({ x, y, 24, 16 }), value(value), texture(NULL), selected(false) {
		refCount = new int;
		refCount[0] = 1;
	}
	Node(const Node<V> &other) : pos(other.pos), value(other.value), refCount(other.refCount), texture(other.texture), selected(other.selected) {
		refCount[0]++;
	}
	~Node() {
		refCount[0]--;
		if (refCount[0] <= 0) {
			SDL_DestroyTexture(texture);
			texture = NULL;
			delete refCount;
		}
	}

	Node<V> &operator=(const Node<V> &other) {
		Node<V> temp = Node<V>(other);
		std::swap(this->pos, temp.pos);
		std::swap(this->value, temp.value);
		std::swap(this->refCount, temp.refCount);
		std::swap(this->texture, temp.texture);
		std::swap(this->selected, temp.selected);
		return *this;
	}

	bool collidesAt(SDL_Point point, SDL_Point offset) {
		SDL_Rect actualPos = { offset.x + pos.x - pos.w / 2, offset.y + pos.y - pos.h / 2, pos.w, pos.h };
		return SDL_PointInRect(&point, &actualPos);
	}
};

template <typename V>
struct Arc {
	bool shortestPath;
	int weight;
	V connectedValue;
	Arc() : shortestPath(false), weight(0), connectedValue(V()) {}
	Arc(int weight, V value) : shortestPath(false), weight(weight), connectedValue(value) {}
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

	Connection<V> &addArc(V value, int weight) { // O(1)-O(N)
		this->connections.put(value, weight);
		return *this;
	}

	Graph<V> &finish() { // O(1)-O(N).
		this->owner->finish(*this);
		return *owner;
	}
};

template <typename V>
class Graph {

	HashMap<V, std::pair<Node<V>, std::vector<Arc<V>>>> nodeMap;

	Node<V> createNode(V value) { // O(1)
		int n = (int)nodeMap.size() + 1;
		int k = (int)ceil((sqrt(n) - 1.0) / 2.0);
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

	Graph<V> &addNode(Node<V> node) { // O(1) - O(N).
		nodeMap.put(node.value, std::make_pair(node, std::vector<Arc<V>>()));
		return *this;
	}

	Connection<V> addValue(V value) { // O(1).
		return Connection<V>(value, this);
	}

	Graph<V> &addArc(V first, V second, int weight) { // O(1) - O(N).
		if (first == second)
			return *this;
		Optional<std::pair<Node<V>, std::vector<Arc<V>>> *> firstEntry = nodeMap.get(first);
		bool found = false;
		for (Arc<V> arc : firstEntry.value->second) {
			if (arc.connectedValue == second) {
				arc.weight = (int)fmin(arc.weight, weight);
				found = true;
			}
		}
		if (!found)
			firstEntry.value->second.push_back(Arc<V>(weight, second));
		std::pair<Node<V>, std::vector<Arc<V>>> &secondEntry = nodeMap.getOrPutIfEmpty(second, std::make_pair(createNode(second), std::vector<Arc<V>>()));
		found = false;
		for (Arc<V> arc : secondEntry.second) {
			if (arc.connectedValue == first) {
				arc.weight = (int)fmin(arc.weight, weight);
				found = true;
			}
		}
		if (!found)
			secondEntry.second.push_back(Arc<V>(weight, first));
		return *this;
	}

	Graph<V> &finish(Connection<V> &connection) { // O(1)-O(N).
		std::vector<Entry<V, int>> arcs;
		connection.connections.getEntries(arcs);
		nodeMap.getOrPutIfEmpty(connection.value, std::make_pair(createNode(connection.value), std::vector<Arc<V>>()));
		for (Entry<V, int> arcData : arcs) {
			addArc(connection.value, arcData.getKey(), arcData.getValue());
		}
		return *this;
	}

	std::unordered_map<V, V> getShortestPath(V start, V end) { // O(1)-O(N)-O(N^2).
		std::unordered_set<V> inSet;
		std::unordered_map<V, int> distances;
		std::unordered_map<V, V> path;
		for (V v : nodeMap.getKeys())
			distances.insert_or_assign(v, INT_MAX);
		distances.insert_or_assign(start, 0);
		inSet.insert(start);
		V currentNode = start;
		while (inSet.find(end) == inSet.end()) { // While the end has not been visited.
			for (Arc<V> arc : nodeMap.get(currentNode).value->second) {
				int tenativeDistance = distances[currentNode] + arc.weight;
				if (tenativeDistance < distances[arc.connectedValue]) {
					distances[arc.connectedValue] = tenativeDistance;
					path.insert_or_assign(arc.connectedValue, currentNode);
				}
			}
			inSet.insert(currentNode);
			bool initialized = false;
			V nextNode;
			for (V nextPossibleNode : nodeMap.getKeys()) {
				if (inSet.find(nextPossibleNode) == inSet.end() && (!initialized || distances[nextNode] > distances[nextPossibleNode])) {
					nextNode = nextPossibleNode;
					initialized = true;
				}
			}
			if (initialized)
				currentNode = nextNode;
			else if (inSet.find(currentNode) == inSet.end())
				return std::unordered_map<V, V>();
		}

		std::unordered_map<V, V> truePath;
		V index = end;
		do {
			truePath.insert_or_assign(path[index], index);
			for (Arc<V> &arc : nodeMap.get(index).value->second)
				if (arc.connectedValue == path[index])
					arc.shortestPath = true;
			for (Arc<V> &arc : nodeMap.get(path[index]).value->second)
				if (arc.connectedValue == index)
					arc.shortestPath = true;
			index = path[index];
		} while (index != start);

		return truePath;
	}

	Graph<V> *getMinimalSpanningTree() { // O(N).
		std::unordered_set<V> inSet;
		if (nodeMap.size() == 0)
			return NULL;
		Graph<V> *spanningTree = new Graph<V>();
		Node<V> firstNode = nodeMap.getValues()[0]->first;
		spanningTree->addNode(Node<V>(firstNode.pos.x, firstNode.pos.y, firstNode.value));
		inSet.insert(firstNode.value);
		for (int i = 0; i < nodeMap.size(); i++) {
			bool initialized = false;
			Arc<V> leastDistance;
			V from;
			for (V v : inSet) {
				for (Arc<V> arc : nodeMap.get(v).value->second) {
					if ((!initialized || leastDistance.weight > arc.weight) && inSet.find(arc.connectedValue) == inSet.end()) {
						initialized = true;
						leastDistance = arc;
						from = v;
					}
				}
			}
			if (initialized) {
				inSet.insert(leastDistance.connectedValue);
				Node<V> temp = nodeMap.get(leastDistance.connectedValue).value->first;
				spanningTree->addNode(Node<V>(temp.pos.x, temp.pos.y, temp.value));
				spanningTree->addArc(from, temp.value, leastDistance.weight);
				spanningTree->addArc(temp.value, from, leastDistance.weight);
			}
		}
		return spanningTree;
	}

	HashMap<V, std::pair<Node<V>, std::vector<Arc<V>>>> getNodeMap() {
		return nodeMap;
	}
};
