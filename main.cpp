#include <iostream>
#include "graphics.h"
#include "input.h"

WindowFrame mainW, spanningW; // The windows.

int nodeId = 0; // The next value for the next node.
std::string newArcValue; // The value for the new arc weight.
Graph<int> mainG, *spanningG; // The main graph and a pointer to a graph that is the minimum spanning tree.
int shortestPath[2] = { -1, -1 }; // The two nodes to get the shortest path from -1 means no node.
std::unordered_map<int, int> graphPath; // The shortest path from where the previous node maps to the next node.
Node<int> *selectedNodes[2]; // Pointers to the two selected nodes.
Arc<int> *changingArcs[2]; // Pointers to the two arcs whose weight is changing. There are two becase each connected node is connected to each other.

bool showUI = true;
int frame = 0;

/*
	TODO: Make Multiple Windows						DONE.
	TODO: Add input to add a Node.					DONE.
	TODO: Add input to make arcs between nodes.		DONE.
	TODO: Add input to change arc weight.			DONE.

	TODO: Add Spanning Tree thingy.					DONE.

	TODO: Add Shortest Path thingy.					DONE.

	TODO: Add Controls UI.							DONE.
	TODO: Add info UI.								DONE.
*/

bool initSDLComponents() {
	// Initialize SDL2.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		std::cout << SDL_GetError() << std::endl;
		return false;
	}
	// Initialize SDL2_ttf.
	if (TTF_Init() == -1) {
		std::cout << SDL_GetError() << std::endl;
		return false;
	}
	// Create two windows.
	bool initialized = mainW.init("Main Graph", 45) && spanningW.init("Minimum Spanning Tree", 45);
	if (initialized) {
		// Set up some initial values.
		int centerX, centerY;
		SDL_GetWindowPosition(mainW.window, &centerX, &centerY);
		SDL_SetWindowPosition(mainW.window, centerX - (16 * 45) / 2 - 15, centerY);
		SDL_SetWindowPosition(spanningW.window, centerX + (16 * 45) / 2 + 15, centerY);
		WindowFrame::FONT = TTF_OpenFont("arial.ttf", 24);
		WindowFrame::xScale = 1.0F;
		WindowFrame::yScale = 1.0F;
		WindowFrame::xOffset = mainW.base.w / 2.0F;
		WindowFrame::yOffset = mainW.base.h / 2.0F;
		WindowFrame::zoom = 1; // Currently unused.
	}
	return initialized;
}

void renderUI() {
	int yOffset = 0;
	if (showUI) { // Render the Controls UI only if we should.
		SDL_DestroyTexture(drawString(mainW.renderer, "Double Left Click to Add a Node.", { 4, yOffset }));
		yOffset += 26;
		SDL_DestroyTexture(drawString(mainW.renderer, "Right Click to Select a node.", { 4, yOffset }));
		yOffset += 26;
		SDL_DestroyTexture(drawString(mainW.renderer, "Right Click on Another Node to Connect Them.", { 4, yOffset }));
		yOffset += 26;
		SDL_DestroyTexture(drawString(mainW.renderer, "Left Click on Another Attached Node to Change the Weight.", { 4, yOffset }));
		yOffset += 26;
		SDL_DestroyTexture(drawString(mainW.renderer, "Left Click and Drag to Move a Selected Node, or the Screen.", { 4, yOffset }));
		yOffset += 26;
		SDL_DestroyTexture(drawString(mainW.renderer, "Left Click on Two Nodes to Find the Shortest Path.", { 4, yOffset }));
		yOffset += 26;
		SDL_DestroyTexture(drawString(mainW.renderer, "<Press Esc to Toggle>", { 4, yOffset }));
		yOffset += 26;
	}
	if (selectedNodes[1] != NULL) { // If the second selected node exists we are changing the weight of the connecting arcs.
		SDL_DestroyTexture(drawString(mainW.renderer, std::string("New Weight: ") + newArcValue + ((frame % 30) - 15 <= 0 ? "_" : ""), { 4, yOffset }));
		yOffset += 26;
	}
	if (shortestPath[0] != -1) { // If the first shortest path exists display the two of them.
		SDL_DestroyTexture(drawString(mainW.renderer, std::string("Start: ") + std::to_string(shortestPath[0]) + (shortestPath[1] != -1 ? std::string(", End: ") + std::to_string(shortestPath[1]) : ", End:  Waiting"), { 4, yOffset }));
	}
}

void handleInput(SDL_Event event) {
	switch (event.type) {
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE) { // Toggle the UI when pressed.
			showUI = !showUI;
		}
		if (selectedNodes[1] != NULL) { // We are changing the weight of arcs.
			if (event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9) { // Only accept 0-9 on the keyboard.
				newArcValue += (char)event.key.keysym.sym;
			} else if (event.key.keysym.sym == SDLK_BACKSPACE && !newArcValue.empty()) { // Backspace removes the last character only if we are not empty.
				newArcValue.pop_back();
			} else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_RETURN2 && !newArcValue.empty()) { // Return means Enter. When it is pressed finish up the changing weights.
				changingArcs[0]->weight = std::stoi(newArcValue);
				changingArcs[1]->weight = std::stoi(newArcValue);
				newArcValue.erase();
				selectedNodes[0]->selected = false;
				selectedNodes[1]->selected = false;
				selectedNodes[0] = NULL;
				selectedNodes[1] = NULL;
				changingArcs[0] = NULL;
				changingArcs[1] = NULL;
			}
		}
		break;
	case SDL_MOUSEMOTION:
		// Store the mouse's position in the base screen coordinates.
		Input::mouse.x = int(event.motion.x * WindowFrame::xScale);
		Input::mouse.y = int(event.motion.y * WindowFrame::yScale);
		if (Input::mouseButtons.isDown(SDL_BUTTON_LEFT)) {
			if (selectedNodes[0] != NULL && selectedNodes[1] == NULL) { // Move the selected node if we aren't changing arc weights.
				selectedNodes[0]->pos.x += int(event.motion.xrel * WindowFrame::xScale);
				selectedNodes[0]->pos.y += int(event.motion.yrel * WindowFrame::yScale);
			} else { // Move the screen otherwise.
				mainW.xOffset += event.motion.xrel * WindowFrame::xScale;
				mainW.yOffset += event.motion.yrel * WindowFrame::yScale;
			}
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		Input::mouseButtons.down(event.button.button);
		if (event.button.button == SDL_BUTTON_LEFT && event.button.clicks == 2) { // If it's a double click add a node.
			mainG.addNode(Node<int>(int(Input::mouse.x - WindowFrame::xOffset), int(Input::mouse.y - WindowFrame::yOffset), nodeId++));
		} else if (event.button.clicks == 1 && event.button.button == SDL_BUTTON_RIGHT) { // If we are a single right click select the first node our mouse collides with.
			bool collision = false;
			for (auto value : mainG.getNodeMap().getValues()) {
				Node<int> &node = value->first;
				if (node.collidesAt(Input::mouse, { int(WindowFrame::xOffset), int(WindowFrame::yOffset) })) {
					if (selectedNodes[0] != NULL && selectedNodes[1] == NULL) {
						mainG.addArc(selectedNodes[0]->value, node.value, 1);
						mainG.addArc(node.value, selectedNodes[0]->value, 1);
					} else {
						if (selectedNodes[0] != NULL)
							selectedNodes[0]->selected = false;
						selectedNodes[0] = &node;
						node.selected = true;
						collision = true;
					}
					break;
				}
			}
			if (!collision && selectedNodes[0] != NULL) {
				selectedNodes[0]->selected = false;
				selectedNodes[0] = NULL;
			}
		} else if (event.button.button == SDL_BUTTON_LEFT) {
			bool collision = false;
			if (selectedNodes[0] != NULL) { // If a node is selected then try to select changing arcs.
				for (auto value : mainG.getNodeMap().getValues()) {
					Node<int> &node = value->first;
					if (&node == selectedNodes[0])
						continue;
					if (node.collidesAt(Input::mouse, { int(WindowFrame::xOffset), int(WindowFrame::yOffset) })) {
						collision = true;
						for (Arc<int> &arc : mainG.getNodeMap().get(selectedNodes[0]->value).value->second) {
							if (arc.connectedValue == node.value)
								changingArcs[0] = &arc;
						}
						for (Arc<int> &arc : mainG.getNodeMap().get(node.value).value->second) {
							if (arc.connectedValue == selectedNodes[0]->value)
								changingArcs[1] = &arc;
						}
						if (changingArcs[0] != NULL && changingArcs[1] != NULL) {
							if (selectedNodes[1] != NULL)
								selectedNodes[1]->selected = false;
							node.selected = true;
							selectedNodes[1] = &node;
						}
						break;
					}
				}
			} else { // Otherwise we are going to find the shortest path?
				for (auto value : mainG.getNodeMap().getValues()) {
					Node<int> &node = value->first;
					if (node.collidesAt(Input::mouse, { int(WindowFrame::xOffset), int(WindowFrame::yOffset) })) {
						collision = true;
						if (shortestPath[0] == -1) {
							shortestPath[0] = node.value;
						} else {
							if (shortestPath[1] != -1) {
								shortestPath[0] = node.value;
								shortestPath[1] = -1;
							} else {
								if (node.value != shortestPath[0]) {
									shortestPath[1] = node.value;
									if (!graphPath.empty()) {
										for (auto pair : graphPath) {
											for (Arc<int> &arc : mainG.getNodeMap().get(pair.first).value->second)
												if (arc.connectedValue == pair.second)
													arc.shortestPath = false;
											for (Arc<int> &arc : mainG.getNodeMap().get(pair.second).value->second)
												if (arc.connectedValue == pair.first)
													arc.shortestPath = false;
										}
										graphPath.clear();
									}
									graphPath = mainG.getShortestPath(shortestPath[0], shortestPath[1]);
								}
							}
						}
					}
				}
			}
			if (!collision && selectedNodes[1] != NULL) {
				selectedNodes[1]->selected = false;
				selectedNodes[1] = NULL;
				changingArcs[0] = NULL;
				changingArcs[1] = NULL;
			}
			if (!collision) {
				shortestPath[0] = -1;
				shortestPath[1] = -1;
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		Input::mouseButtons.up(event.button.button);
		break;
	}
}

int main(int argc, char *argv[]) {
	if (!initSDLComponents())
		return -1;
	bool running = true;
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) { // If we click either close button end the program.
				running = false;
				break;
			}
			switch (event.type) {
			case SDL_WINDOWEVENT:// Pass the events off to the WindowFrames.
				mainW.handleEvents(event);
				spanningW.handleEvents(event);
				switch (event.window.event) { // If the event was a resize even then update our scale factors.
				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					if (event.window.windowID == mainW.windowID) {
						WindowFrame::xScale = float(mainW.base.w) / float(mainW.actual.w);
						WindowFrame::yScale = float(mainW.base.h) / float(mainW.actual.h);
					}
				}
				break;
			default:
				if (event.window.windowID == mainW.windowID) // Only handle use input on the main window.
					handleInput(event);
			}
		}
		SDL_SetRenderDrawColor(mainW.renderer, 255, 255, 255, 255); // Reset main window drawing color to white.
		drawGraph(mainW, mainG); // Draw the main graph on the main display.
		mainW.display(); // Display the main window frame..
		SDL_SetRenderDrawColor(spanningW.renderer, 255, 255, 255, 255); // Reset spanning tree window drawing color to white.
		spanningG = mainG.getMinimalSpanningTree(); // Get the minimum spanning tree of the main graph.
		if (spanningG != NULL) { // If the spanning tree isn't null draw it on the spanning tree window and then free it's memory.
			drawGraph(spanningW, *spanningG);
			delete spanningG;
			spanningG = NULL;
		}
		spanningW.display(); // Display the spanning tree window frame.

		renderUI(); // Render the UI on top of the graphs.
		frame++; // Increment the frame counter;

		SDL_Delay(16); // Delay for 16 milliseconds.
	}

	mainW.destroy(); // Destroy the window frames.
	spanningW.destroy();
	TTF_Quit(); // Exit out of the SDL components.
	SDL_Quit();
	return 0;
}
