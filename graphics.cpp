#include <iostream>
#include "graphics.h"

TTF_Font *WindowFrame::FONT = NULL;
float WindowFrame::xOffset = 0.0F, WindowFrame::yOffset = 0, WindowFrame::xScale = 1.0F, WindowFrame::yScale = 1.0F;
int WindowFrame::zoom = 1;

bool WindowFrame::init(const char *title, int scale) {
	// Create an SDL_Window if it fails print the error.
	if ((window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 16 * scale, 9 * scale, SDL_WINDOW_RESIZABLE)) == 0) {
		std::cout << SDL_GetError() << std::endl;
		return false;
	}
	// Create an SDL_Renderer for the window if it fails print the error.
	if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_ACCELERATED)) == 0) {
		std::cout << SDL_GetError() << std::endl;
		return false;
	}
	// Create a buffer texture to draw to before the the window.
	texture = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_TARGET, 16 * scale, 9 * scale);
	SDL_SetRenderTarget(renderer, texture);
	SDL_GetWindowSize(window, &base.w, &base.h);
	SDL_GetWindowSize(window, &actual.w, &actual.h);
	windowID = SDL_GetWindowID(window);
	return true;
}

void WindowFrame::display() {
	// Switch to the window as the render target, copy the texture over and then display the window and switch back to the texture and clear the texture.
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, texture, &base, &actual);
	SDL_RenderPresent(renderer);
	SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, texture);
	SDL_RenderClear(renderer);
}

void WindowFrame::handleEvents(SDL_Event event) {
	if (event.window.windowID != windowID)
		return;
	switch (event.window.event) {
	case SDL_WINDOWEVENT_RESIZED: // Update the actual SDL_Rect to have the windows new width and height.
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		SDL_GetWindowSize(window, &actual.w, &actual.h);
		break;
	default:
		break;
	}
}

void WindowFrame::destroy() { // Destroy the SDL objects.
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(texture);
}

SDL_Texture *drawString(SDL_Renderer *renderer, std::string text, SDL_Point pos, bool draw, SDL_Rect *srcRect, bool center, SDL_Color color) {
	SDL_Texture *texture;
	SDL_Surface *surface = TTF_RenderText_Solid(WindowFrame::FONT, text.c_str(), color);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect dest = { pos.x, pos.y, surface->w, surface->h };
	if (center) {
		dest.x -= dest.w / 2;
		dest.y -= dest.h / 2;
	}
	if (srcRect != NULL)
		*srcRect = { srcRect->x, srcRect->y, surface->w, surface->h };
	if (draw)
		SDL_RenderCopy(renderer, texture, NULL, &dest);
	SDL_FreeSurface(surface);
	return texture;
}
