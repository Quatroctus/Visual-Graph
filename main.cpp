#include <iostream>
#include <vector>
#include "SDL.h"
#include <SDL_ttf.h>
#include "graphics.h"
#include "input.h"

SDL_Rect base, actual;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *drawingTexture;

Graph<int> graph;
Node<int> *activeNode = NULL;

constexpr int WIDTH = 16 * 50, HEIGHT = 9 * 50;

float Camera::xScale = 1.0F, Camera::yScale = 1.0F;
float Camera::xOffset = WIDTH / 2, Camera::yOffset = HEIGHT / 2;
int Camera::zoom = 1;

void handleInputEvents(SDL_Event ev) {
	switch (ev.type) {
	case SDL_MOUSEWHEEL:
		Input::scroll += ev.wheel.y;
		break;
	case SDL_MOUSEBUTTONDOWN:
		Input::mouseButtons.down(ev.button.button);
		if (Input::mouseButtons.isDown(SDL_BUTTON_RIGHT)) {
			SDL_Point mouse = { (int)(float(Input::mouseX) - Camera::xOffset), (int)(float(Input::mouseY) - Camera::yOffset) };
			std::vector<Entry<int, std::pair<Node<int>, std::vector<Arc<int>>>>> entries;
			graph.getNodeMap().getEntries(entries);
			for (auto entry : entries) {
				Node<int> &node = entry.getValue().first;
				SDL_Rect renderedRect = { node.pos.x - (node.pos.w / 2), node.pos.y - (node.pos.h / 2), node.pos.w, node.pos.h };
				if (SDL_PointInRect(&mouse, &renderedRect))
					activeNode = &node;
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		Input::mouseButtons.up(ev.button.button);
		if (Input::mouseButtons.isUp(SDL_BUTTON_RIGHT)) {
			Input::relX = 0.0F;
			Input::relY = 0.0F;
			activeNode = NULL;
		}
		break;
	case SDL_KEYDOWN:
		Input::keys.down(ev.key.keysym.sym);
		break;
	case SDL_KEYUP:
		Input::keys.up(ev.key.keysym.sym);
		break;
	case SDL_MOUSEMOTION:
		if (activeNode != NULL) {
			Input::relX += ev.motion.xrel * Camera::xScale;
			Input::relY += ev.motion.yrel * Camera::yScale;
			activeNode->pos.x += int(Input::relX);
			activeNode->pos.y += int(Input::relY);
			Input::relX -= int(Input::relX);
			Input::relY -= int(Input::relY);
		}
		Input::mouseX = (int) (float(ev.motion.x) * Camera::xScale);
		Input::mouseY = (int)(float(ev.motion.y) * Camera::yScale);
		if (Input::mouseButtons.isDown(SDL_BUTTON_LEFT)) {
			Camera::xOffset += ev.motion.xrel * Camera::xScale;
			Camera::yOffset += ev.motion.yrel * Camera::yScale;
		}
		break;
	}
}

void handleEvent(SDL_Event ev) {
	switch (ev.type) {
	case SDL_WINDOWEVENT: {
		switch (ev.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			SDL_GetWindowSize(window, &actual.w, &actual.h);
			Camera::xScale = (float)base.w / (float)actual.w;
			Camera::yScale = (float)base.h / (float)actual.h;
			break;
		}
	} break;
	default:
		handleInputEvents(ev);
	}
}

int main(int argc, char *argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) > 0) {
		std::cout << SDL_GetError() << std::endl;
		return -1;
	}
	std::cout << "SDL_INITIALIZED." << std::endl;
	if (!(window = SDL_CreateWindow("Visual Graphs", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE))) {
		std::cout << SDL_GetError() << std::endl;
		return -2;
	}
	SDL_GetWindowSize(window, &base.w, &base.h);
	SDL_GetWindowSize(window, &actual.w, &actual.h);
	std::cout << "WINDOW_INITIALIZED." << std::endl;
	if (!(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED))) {
		std::cout << SDL_GetError() << std::endl;
		return -3;
	}
	std::cout << "RENDERER_INITIALIZED." << std::endl;
	drawingTexture = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
	SDL_SetRenderTarget(renderer, drawingTexture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
	TTF_Init();

	graph.addValue(0).addArc(1, 10).finish();
	graph.addValue(10).addArc(1, 20).addArc(0, 15).finish();
	graph.addValue(20).addArc(0, 5).finish();
	graph.addValue(30).addArc(20, 1).addArc(1, 100).finish();
	graph.addValue(100).finish();
	
	bool running = true;
	while (running) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT:
				running = false;
				break;
			default:
				handleEvent(ev);
			}
		}

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		drawGraph(renderer, graph);

		SDL_SetRenderTarget(renderer, NULL);
		SDL_RenderCopy(renderer, drawingTexture, &base, &actual);
		SDL_RenderPresent(renderer);
		SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderTarget(renderer, drawingTexture);
		SDL_RenderClear(renderer);

		SDL_Delay(16);
	}
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(drawingTexture);
	SDL_Quit();
	return 0;
}
