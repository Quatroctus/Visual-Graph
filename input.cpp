#include "input.h"

int Input::scroll = 0, Input::mouseX = 0, Input::mouseY = 0, Input::relX = 0, Input::relY = 0;
InputMap Input::keys = InputMap();
InputMap Input::mouseButtons = InputMap();

void InputMap::down(int id) {
	this->map.insert_or_assign(id, true);
}

void InputMap::up(int id) {
	this->map.insert_or_assign(id, false);
}

void InputMap::set(int id, bool state) {
	this->map.insert_or_assign(id, state);
}

bool InputMap::isDown(int id) {
	auto value = this->map.find(id);
	if (value != this->map.end())
		return value->second;
	return false;
}

bool InputMap::isUp(int id) {
	auto value = this->map.find(id);
	if (value != this->map.end())
		return value->second;
	return false;
}