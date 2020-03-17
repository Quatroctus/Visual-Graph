#pragma once
#include <SDL_rect.h>
#include <unordered_map>

// Simple input stuff. Everything is constant time.

class InputMap {

	std::unordered_map<int, bool> map;

public:
	void down(int id);
	void up(int id);
	void set(int id, bool state);

	bool isDown(int id);
	bool isUp(int id);
};

struct Input {
	static int scroll;
	static SDL_Point mouse;
	static InputMap mouseButtons;
	static InputMap keys;
};
