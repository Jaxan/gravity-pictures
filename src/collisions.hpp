#pragma once

#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

#define _LIBCPP_STD_VER 13 // I AM A LEET HAXORX, should update my clang though...
#include <optional>

#include "math_ext.hpp"

// starting position and direction
// some often used values are precomputed
template <typename T>
struct line{
	using vec2 = math::vector<T, 2>;
	vec2 position;
	vec2 direction;

	line() = default;
	line(line const &) = default;
	line& operator=(line const &) = default;

	line(vec2 position, vec2 direction)
	: position(position), direction(direction)
	{ update_stuff(); }

	void update_stuff(){
		lengthsqr = math::dot(direction, direction);
		normal = math::normalize(math::rotate_ccw(direction));
		crosslength = math::cross_length(position, direction);
	}

	vec2 normal;
	T lengthsqr;
	T crosslength;
};

// Intersection of general parabola and line (oriented). Returns all (times of) intersections.
template <typename T>
std::vector<T> check_collision(math::vector<T, 2> acc, math::vector<T, 2> vel, math::vector<T, 2> pc, line<T> line){
	using namespace math;
	// compute 1 quadratic thing
	auto a = T{0.5} * cross_length(acc, line.direction);
	auto b = cross_length(vel, line.direction);
	auto c = cross_length(pc, line.direction) - line.crosslength;

	// in case there is no acc
	if(a == 0) return{-c / b};

	// calculate discriminant
	auto d = b*b - 4*a*c;
	if(d < 0) return {};

	d = std::sqrt(d);

	// solve
	auto t1 = (-b + d) / (2 * a);
	auto t2 = (-b - d) / (2 * a);

	if(t1 == t2) return {t1};
	return {t1, t2};
}

// check whether position lies on line segment, instead of infi line
template <typename T>
bool check_range(math::vector<T, 2> position, line<T> const & l){
	auto d = math::dot(position - l.position, l.direction) / math::dot(l.direction, l.direction);
	return 0 <= d && d <= 1;
}

// check for 1-direction lines
template <typename T>
bool check_velocity(math::vector<T, 2> velocity, line<T> const & l){
	return math::dot(velocity, l.normal) < 0;
}

// information of a collision, will generally not be calculated at once
template <typename T>
struct collision_info{
	using vec2 = math::vector<T, 2>;
	vec2 position;
	vec2 velocity;
	line<T> line;
	T time{0.0};
};

// position of bounce, and velocity out of it
template <typename T>
struct bounce{
	using vec2 = math::vector<T, 2>;
	vec2 position;
	vec2 velocity;
	T time{0.0f};

	bounce(vec2 position, vec2 velocity, T time = 0.0f)
	: position(position), velocity(velocity), time(time)
	{}
};

// container for options
template <typename T>
struct trace_options{
	math::vector<T, 2> gravity {0.0, -10.0};
	size_t max_bounces {500};
	T max_time {30};
};

// container for lines
template <typename T>
using lines = std::vector<line<T>>;

// make a trace of all bounces
template <typename T>
std::vector<bounce<T>> trace(trace_options<T> const & options, bounce<T> b, lines<T> const & lines){
	// start at the beginning :>
	std::vector<bounce<T>> bounces{b};

	for(int i = 0; i < options.max_bounces; ++i){
		std::optional<collision_info<T>> ci;

		// for each line, determine time of bounce, record the earliest
		for(auto&& l : lines){
			for(auto&& t : check_collision(options.gravity, b.velocity, b.position, l)){
				// time of collision
				if(t <= 0 || (ci && t >= ci->time) || b.time + t > options.max_time) continue;

				// velocity towards collision
				auto velocity = options.gravity*t + b.velocity;
				if(!check_velocity(velocity, l)) continue;

				// position of collision
				auto position = T{0.5}*options.gravity*t*t + b.velocity*t + b.position;
				if(!check_range(position, l)) continue;

				// FIXME: use .emplace if that works (bug in clang of libc++?)
				ci = collision_info<T>{position, velocity, l, t};
			}
		}

		// no bounce found, hence we can stop
		if(!ci) break;

		// calculate new direction
		b.position = ci->position;
		b.velocity = T{-2.0} * math::dot(ci->velocity, ci->line.normal) * ci->line.normal + ci->velocity;
		
		// keep track of global time
		b.time += ci->time;

		// done
		bounces.push_back(b);
	}

	return bounces;
}
