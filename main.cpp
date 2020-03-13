#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include "graphics.h"
#include "input.h"

SDL_Rect base, actual;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *drawingTexture;

Graph<int> graph;
std::string changingValue;
bool beginkeyboardListening = false;
Arc<int> *changingArcs[2];
Node<int> *motionNode = NULL, *selectedNode = NULL, *connectedNode = NULL;
int nodeId = 0;
unsigned int lastPressed;

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
					motionNode = &node;
			}
		}
		break;
	case SDL_MOUSEBUTTONUP: {
		Input::mouseButtons.up(ev.button.button);
		if (ev.button.button == SDL_BUTTON_RIGHT) {
			Input::relX = 0.0F;
			Input::relY = 0.0F;
			motionNode = NULL;
		} else if (ev.button.button == SDL_BUTTON_LEFT) {
			SDL_Point mouse = { (int)(float(Input::mouseX) - Camera::xOffset), (int)(float(Input::mouseY) - Camera::yOffset) };
			std::vector<Entry<int, std::pair<Node<int>, std::vector<Arc<int>>>>> entries;
			graph.getNodeMap().getEntries(entries);
			bool noCollision = true;
			for (auto entry : entries) {
				Node<int> &node = entry.getValue().first;
				SDL_Rect renderedRect = { node.pos.x - (node.pos.w / 2), node.pos.y - (node.pos.h / 2), node.pos.w, node.pos.h };
				if (SDL_PointInRect(&mouse, &renderedRect)) {
					if (Input::keys.isDown(SDLK_LCTRL) || Input::keys.isDown(SDLK_RCTRL)) {
						if (selectedNode != NULL) {
							if (connectedNode != NULL) {
								connectedNode->selected = false;
							}
							for (Arc<int> &arc : graph.getNodeMap().get(node.value).value->second)
								if (arc.connectedValue == selectedNode->value)
									changingArcs[0] = &arc;
							for (Arc<int> &arc : graph.getNodeMap().get(selectedNode->value).value->second)
								if (arc.connectedValue == node.value)
									changingArcs[1] = &arc;
							beginkeyboardListening = changingArcs[0] != NULL && changingArcs[1] != NULL;
							if (beginkeyboardListening) {
								connectedNode = &node;
								node.selected = true;
								noCollision = false;
							}
						}
					} else if (selectedNode != NULL) {
						graph.addArc(selectedNode->value, node.value, 1);
						graph.addArc(node.value, selectedNode->value, 1);
						selectedNode->selected = false;
						selectedNode = NULL;
					} else {
						selectedNode = &node;
						node.selected = true;
						noCollision = false;
					}
					break;
				}
			}
			if (noCollision) {
				if (selectedNode != NULL) {
					selectedNode->selected = false;
					selectedNode = NULL;
				}
				if (connectedNode != NULL) {
					connectedNode->selected = false;
					connectedNode = NULL;
				}
			}
		}
		unsigned int current = SDL_GetTicks();
		if (current - lastPressed < 200 && ev.button.button == SDL_BUTTON_LEFT)
			graph.addValue(nodeId++).finish();
		lastPressed = SDL_GetTicks();
	} break;
	case SDL_KEYDOWN:
		Input::keys.down(ev.key.keysym.sym);
		break;
	case SDL_KEYUP:
		Input::keys.up(ev.key.keysym.sym);
		if (beginkeyboardListening && ev.key.keysym.sym >= SDLK_0 && ev.key.keysym.sym <= SDLK_9) {
			changingValue += (char)ev.key.keysym.sym;
		} else if (beginkeyboardListening && ev.key.keysym.sym == SDLK_BACKSPACE)
			changingValue.pop_back();
		else if (beginkeyboardListening && ev.key.keysym.sym == SDLK_RETURN) {
			changingArcs[0]->weight = std::stoi(changingValue);
			changingArcs[1]->weight = std::stoi(changingValue);
			changingValue.clear();
			selectedNode->selected = false;
			connectedNode->selected = false;
			selectedNode = NULL;
			connectedNode = NULL;
			changingArcs[0] = NULL;
			changingArcs[1] = NULL;
			beginkeyboardListening = false;
		}
		break;
	case SDL_MOUSEMOTION:
		if (motionNode != NULL) {
			Input::relX += ev.motion.xrel * Camera::xScale;
			Input::relY += ev.motion.yrel * Camera::yScale;
			motionNode->pos.x += int(Input::relX);
			motionNode->pos.y += int(Input::relY);
			Input::relX -= int(Input::relX);
			Input::relY -= int(Input::relY);
		}
		Input::mouseX = (int)(float(ev.motion.x) * Camera::xScale);
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

void drawChangingArcValue() {
	if (!changingValue.empty()) {
		if (FONT == NULL)
			FONT = TTF_OpenFont("arial.ttf", 24);
		SDL_Surface *surface;
		SDL_Texture *texture;
		SDL_Color textColor = { 255, 255, 255, 0 };
		surface = TTF_RenderText_Solid(FONT, changingValue.c_str(), textColor);
		texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_Rect dest = { 10, 10, surface->w, surface->h };
		SDL_RenderCopy(renderer, texture, NULL, &dest);
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);
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
		drawChangingArcValue();

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
