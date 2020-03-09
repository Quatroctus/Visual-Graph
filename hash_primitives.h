#pragma once
#define DECL_PRIMITIVE_HASH(x) template<> struct primitive<x> { static constexpr bool exists() { return true; } static const size_t hash(x val) { return size_t(val); } };
template <typename T> struct primitive {
	static constexpr bool exists() { return false; }
	static const size_t hash(T val) { return 0; }
};

DECL_PRIMITIVE_HASH(char);
DECL_PRIMITIVE_HASH(wchar_t);
DECL_PRIMITIVE_HASH(short);
DECL_PRIMITIVE_HASH(int);
DECL_PRIMITIVE_HASH(float);
DECL_PRIMITIVE_HASH(double)
DECL_PRIMITIVE_HASH(long);

DECL_PRIMITIVE_HASH(unsigned char);
DECL_PRIMITIVE_HASH(unsigned short);
DECL_PRIMITIVE_HASH(unsigned int);
DECL_PRIMITIVE_HASH(unsigned long);

DECL_PRIMITIVE_HASH(long double)
DECL_PRIMITIVE_HASH(long long);

DECL_PRIMITIVE_HASH(unsigned long long);
