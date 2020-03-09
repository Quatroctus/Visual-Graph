#pragma once
#include <unordered_map>

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
	static int scroll, mouseX, mouseY, relX, relY;
	static InputMap mouseButtons;
	static InputMap keys;
};
