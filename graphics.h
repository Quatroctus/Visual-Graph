#pragma once
#include "SDL.h"
#include "graph.h"

struct Camera {
	static float xScale, yScale;
	static float xOffset, yOffset;
	static int zoom;
};

template <typename V>
void drawGraph(SDL_Renderer *renderer, Graph<V> graph) {
	std::vector<KeyValue<V, std::pair<Node<V>, std::vector<Arc<V>>>>> entries;
	graph.getNodeMap().getEntries(&entries);
	for (auto entry : entries) {
		drawNode(renderer, entry.getValue().first);
		for (Arc<V> arc : entry.getValue().second) {
			drawArc(renderer, entry.getValue().first, graph.getNodeMap().query(arc.connectedValue).value.first);
		}
	}
}

template <typename V>
void drawNode(SDL_Renderer *renderer, Node<V> node) {
	SDL_Rect rect = { node.pos.x + (int) Camera::xOffset - node.pos.w / 2, node.pos.y + (int) Camera::yOffset - node.pos.h / 2, node.pos.w, node.pos.h };
	SDL_RenderDrawRect(renderer, &rect);
}

template <typename V>
void drawArc(SDL_Renderer *renderer, Node<V> n1, Node<V> n2) {
	SDL_RenderDrawLine(renderer, n1.pos.x + (int) Camera::xOffset, n1.pos.y + (int) Camera::yOffset, n2.pos.x + (int) Camera::xOffset, n2.pos.y + (int) Camera::yOffset);
}
