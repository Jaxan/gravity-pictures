#pragma once

#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

#include "math_ext.hpp"

// time of collision, and some debugging info
template <typename T>
struct collision_info{
	enum collision_type {
		normal,
		no_coll_discr,
		no_coll_time
	} type{no_coll_time};
	math::vector<T, 2> position;
	T time{0.0};

	collision_info() = default;

	collision_info(collision_type t)
	: type(t)
	{ assert(type != normal); }

	collision_info(T t)
	: time(t)
	{ type = normal; }

	bool valid() const {
		return type == normal;
	}
};

template <typename T>
std::ostream& operator<<(std::ostream& out, collision_info<T> const & ci){
	switch(ci.type){
		case collision_info<T>::normal : return out << "{ collision at " << ci.time << " }";
		case collision_info<T>::no_coll_discr : return out << "{ no collision at all }";
		case collision_info<T>::no_coll_time : return out << "{ no collision in future }";
	}
}

// starting position and direction, also indicating length
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

// intersection of general parabola and line (oriented)
template <typename T>
collision_info<T> check_collision(math::vector<T, 2> acc, math::vector<T, 2> vel, math::vector<T, 2> pc, line<T> line){
	using namespace math;
	// compute 1 quadratic thing
	T a = T{0.5} * cross_length(acc, line.direction);
	T b = cross_length(vel, line.direction);
	T c = cross_length(pc, line.direction) - line.crosslength;

	// calculate discriminant
	T d = b*b - 4*a*c;
	if(d < 0) return {collision_info<T>::no_coll_discr};

	d = std::sqrt(d);

	// solve
	T t1 = (-b + d) / (2 * a);
	T t2 = (-b - d) / (2 * a);

	// in case there is no acc
	if(a == 0) t1 = t2 = -c / b;

	// pick closest
	if(t1 > t2 && t2 > 0) t1 = t2;
	if(t1 <= 0) return {collision_info<T>::no_coll_time};

	return {t1};
}

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

template <typename T>
bool check_range(math::vector<T, 2> position, line<T> const & l){
	auto d = math::dot(position - l.position, l.direction) / math::dot(l.direction, l.direction);
	return 0 <= d && d <= 1;
}

template <typename T>
std::vector<bounce<T>> trace(math::vector<T, 2> gravity, bounce<T> b, std::vector<line<T>> const & lines){
	std::vector<bounce<T>> bounces{b};
	for(int i = 0; i < 5; ++i){
		collision_info<T> ci;
		ci.time = std::numeric_limits<T>::max();
		line<T> cl;

		// for each line, determine time of bounce, record the earliest
		for(auto&& l : lines){
			auto ci2 = check_collision(gravity, b.velocity, b.position, l);
			if(ci2.valid() && ci2.time < ci.time){
				auto t = ci2.time;
				auto position = T{0.5}*gravity*t*t + b.velocity*t + b.position;
				if(check_range(position, l)){
					ci = ci2;
					ci.position = position;
					cl = l;
				}
			}
		}

		std::cout << ci << std::endl;

		// no bounce found, hence we can stop
		if(!ci.valid()) break;

		// calculate position of bounce, and velocity into it
		auto t = ci.time;
		auto velocity = gravity*t + b.velocity;
		auto normal = math::rotate_ccw(math::normalize(cl.direction));

		// calculate new direction
		b.position = ci.position;
		b.velocity = T{-2.0} * math::dot(velocity, normal) * normal + velocity;
		b.time += t;

		bounces.push_back(b);
	}
	return bounces;
}
