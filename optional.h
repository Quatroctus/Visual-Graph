#pragma once
template <typename V>
struct Optional {
	bool exists;
	V value;
	Optional() : exists(false), value(V()) {}
	Optional(V value) : exists(true), value(value) {}
};
