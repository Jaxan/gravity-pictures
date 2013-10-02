#pragma once

#include <iostream>
#include "math.hpp"

template <typename T, int N>
std::ostream& operator<<(std::ostream& out, math::vector<T, N> v){
	T const (&v2)[N] = reinterpret_cast<T const (&)[N]>(v);
	out << "{ ";
	for(auto && x : v2){
		out << x << " ";
	}
	return out << "}";
}
