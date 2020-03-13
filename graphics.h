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
			drawArc(renderer, entry.getValue().first, graph.getNodeMap().get(arc.connectedValue).value->first, arc.weight);
		}
	}
}

template <typename V>
void drawNode(SDL_Renderer *renderer, Node<V> &node) {
	SDL_Rect rect = { node.pos.x + (int) Camera::xOffset - node.pos.w / 2, node.pos.y + (int) Camera::yOffset - node.pos.h / 2, node.pos.w, node.pos.h };
	
	if (node.selected) {
		SDL_SetRenderDrawColor(renderer, 10, 255, 100, 255);
		SDL_RenderDrawRect(renderer, &rect);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	} else
		SDL_RenderDrawRect(renderer, &rect);
	if (FONT == NULL)
		FONT = TTF_OpenFont("arial.ttf", 24);
	if (node.texture == NULL) {
		SDL_Surface *surface;
		SDL_Color textColor = { 255, 255, 255, 0 };
		std::string text = std::to_string(node.value);
		surface = TTF_RenderText_Solid(FONT, text.c_str(), textColor);
		node.texture = SDL_CreateTextureFromSurface(renderer, surface);
		node.pos.w = surface->w + 12;
		node.pos.h = surface->h + 8;
		SDL_FreeSurface(surface);
	}
	SDL_RenderCopy(renderer, node.texture, NULL, &rect);

}

template <typename V>
void drawArc(SDL_Renderer *renderer, Node<V> n1, Node<V> n2, int weight) {
	SDL_RenderDrawLine(renderer, n1.pos.x + (int) Camera::xOffset, n1.pos.y + (int) Camera::yOffset, n2.pos.x + (int) Camera::xOffset, n2.pos.y + (int) Camera::yOffset);
	
	if (FONT == NULL)
		FONT = TTF_OpenFont("arial.ttf", 24);
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Color textColor = { 255, 255, 255, 0 };
	std::string text = std::to_string(weight);
	surface = TTF_RenderText_Solid(FONT, text.c_str(), textColor);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect dest = { (int) Camera::xOffset + n1.pos.x + ((n2.pos.x - n1.pos.x) / 2), (int) Camera::yOffset + n1.pos.y + ((n2.pos.y - n1.pos.y) / 2), surface->w, surface->h }; // TODO
	SDL_RenderCopy(renderer, texture, NULL, &dest);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);

}
