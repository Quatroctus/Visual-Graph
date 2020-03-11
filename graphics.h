#pragma once
#include <string>
#include "SDL.h"
#include "graph.h"

static TTF_Font *FONT = NULL;

struct Camera {
	static float xScale, yScale;
	static float xOffset, yOffset;
	static int zoom;
};

template <typename V>
void drawGraph(SDL_Renderer *renderer, Graph<V> graph) {
	std::vector<Entry<V, std::pair<Node<V>, std::vector<Arc<V>>>>> entries;
	graph.getNodeMap().getEntries(entries);
	for (auto entry : entries) {
		drawNode(renderer, entry.getValue().first);
		for (Arc<V> arc : entry.getValue().second) {
			drawArc(renderer, entry.getValue().first, graph.getNodeMap().get(arc.connectedValue).value->first);
		}
	}
}

template <typename V>
void drawNode(SDL_Renderer *renderer, Node<V> &node) {
	SDL_Rect rect = { node.pos.x + (int) Camera::xOffset - node.pos.w / 2, node.pos.y + (int) Camera::yOffset - node.pos.h / 2, node.pos.w, node.pos.h };
	SDL_RenderDrawRect(renderer, &rect);
	if (FONT == NULL)
		FONT = TTF_OpenFont("arial.ttf", 24);
	if (node.texture == NULL) {
		SDL_Surface *surface;
		SDL_Color textColor = { 255, 255, 255, 0 };
		std::string text = std::to_string(node.value);
		std::cout << text.c_str() << std::endl;
		surface = TTF_RenderText_Solid(FONT, text.c_str(), textColor);
		node.texture = SDL_CreateTextureFromSurface(renderer, surface);
		node.pos.w = surface->w + 12;
		node.pos.h = surface->h + 8;
		SDL_FreeSurface(surface);
	}
	SDL_RenderCopy(renderer, node.texture, NULL, &rect);

}

template <typename V>
void drawArc(SDL_Renderer *renderer, Node<V> n1, Node<V> n2) {
	SDL_RenderDrawLine(renderer, n1.pos.x + (int) Camera::xOffset, n1.pos.y + (int) Camera::yOffset, n2.pos.x + (int) Camera::xOffset, n2.pos.y + (int) Camera::yOffset);
}
