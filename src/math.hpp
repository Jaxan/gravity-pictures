#pragma once

#include <cmath>

/*	Using built-in clang vector support
	I expect it to be working on gcc as well */

namespace math{
	// The types
	template <typename T, int N>
	using vector = T __attribute__((ext_vector_type(N)));

	template <typename VT>
	struct scalar_{};

	// Extracting the scalar type from an vector
	template <typename T, int N>
	struct scalar_<vector<T, N>> {
		using type = T;
	};

	template <typename T>
	using scalar = typename scalar_<T>::type;

	// Calculations
	template <typename T>
	T cross_length(vector<T, 2> a, vector<T, 2> b){
		return a.x * b.y - a.y * b.x;
	}

	template <typename T>
	T dot(vector<T, 2> a, vector<T, 2> b){
		return a.x * b.x + a.y + b.y;
	}

	template <typename T, int N>
	vector<T, N> normalize(vector<T, N> a){
		return a / std::sqrt(dot(a, a));
	}

	template <typename T>
	vector<T, 2> rotate_ccw(vector<T, 2> v){
		v = v.yx;
		v.x *= -1.0;
		return v;
	}
}
