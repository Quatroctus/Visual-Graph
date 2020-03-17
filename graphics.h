#pragma once
#include <string>
#include "SDL.h"
#include <SDL_ttf.h>
#include "graph.h"

struct WindowFrame {
	static TTF_Font *FONT;
	static float xScale, yScale, xOffset, yOffset;
	static int zoom;
	int windowID;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Rect base, actual;

	bool init(const char *title, int scale);
	void display();
	void handleEvents(SDL_Event event);
	void destroy();

};

template <typename V>
void drawGraph(WindowFrame frame, Graph<V> graph) {
	std::vector<Entry<V, std::pair<Node<V>, std::vector<Arc<V>>>>> entries;
	graph.getNodeMap().getEntries(entries);
	for (auto entry : entries) {
		SDL_SetRenderDrawColor(frame.renderer, 255, 255, 255, 255);
		drawNode(frame, entry.getValue().first); // Draw the node.
		for (Arc<V> arc : entry.getValue().second) {
			if (arc.shortestPath) // Set the color to blue if this arc is part of the shortest path.
				SDL_SetRenderDrawColor(frame.renderer, 10, 100, 255, 255);
			else // Set the color to white if this arc is not part of the shortest path.
				SDL_SetRenderDrawColor(frame.renderer, 255, 255, 255, 255);
			// Draw the arc.
			drawArc(frame, entry.getValue().first, graph.getNodeMap().get(arc.connectedValue).value->first, arc.weight);
		}
	}
}

template <typename V>
void drawNode(WindowFrame frame, Node<V> &node) {
	if (node.texture == NULL) { // If we have no texture make one.
		node.texture = drawString(frame.renderer, std::to_string(node.value), { -1, -1 }, false, &node.pos);
		node.pos.w += 12;
		node.pos.h += 8;
	}
	SDL_Rect rect = { node.pos.x + (int)frame.xOffset - node.pos.w / 2, node.pos.y + (int)frame.yOffset - node.pos.h / 2, node.pos.w, node.pos.h };
	if (node.selected) { // Render a green rect if we are selected.
		SDL_SetRenderDrawColor(frame.renderer, 10, 255, 100, 255);
		SDL_RenderDrawRect(frame.renderer, &rect);
		SDL_SetRenderDrawColor(frame.renderer, 255, 255, 255, 255);
	} else // Render a hopefully white rect if we are not selected.
		SDL_RenderDrawRect(frame.renderer, &rect);
	// Render the value texture.
	SDL_RenderCopy(frame.renderer, node.texture, NULL, &rect);
}

template <typename V>
void drawArc(WindowFrame frame, Node<V> n1, Node<V> n2, int weight) {
	// Render a straight line from n1 to n2.
	SDL_RenderDrawLine(frame.renderer, n1.pos.x + (int)frame.xOffset, n1.pos.y + (int)frame.yOffset, n2.pos.x + (int)frame.xOffset, n2.pos.y + (int)frame.yOffset);
	// Render the weight and destroy the created texture.
	SDL_DestroyTexture(drawString(frame.renderer, std::to_string(weight), { (int)frame.xOffset + n1.pos.x + ((n2.pos.x - n1.pos.x) / 2), (int)frame.yOffset + n1.pos.y + ((n2.pos.y - n1.pos.y) / 2) }, true, NULL, true));
}

SDL_Texture *drawString(SDL_Renderer *renderer, std::string text, SDL_Point pos, bool draw=true, SDL_Rect *srcRect=NULL, bool center=false, SDL_Color color={ 255, 255, 255, 255 });
