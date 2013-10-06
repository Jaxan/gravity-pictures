#pragma once

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>

#include <ImageStreams/include/png.hpp>
#include <moggle/math/projection.hpp>

#include "opengl.hpp"
#include "shader.hpp"
#include "fbo.hpp"
#include "renderoptions.hpp"

#include "collisions.hpp"
using namespace math;
using T = double;
using Vec2 = math::vector<T, 2>;

struct Color {
	unsigned char r, g, b;
	Color(unsigned char x) : r(x), g(x), b(x) {}
	Color(unsigned char r, unsigned int g, unsigned int b) : r(r), g(g), b(b) {}
};

struct LineVertex {
	Vec2 position;
	Color color;

	LineVertex(Vec2 p, unsigned char x) : position(p), color(x) {}
};

struct AppOptions {
	float dt{1.0f / 20.0f};
	float line_length{200.0f};
	int number_of_lines{50};
	int line_bound{5000};

	float exposure{1.5};	// higher is lighter
	float gamma{2.0};	// lower is lighter
};

struct app{
	RenderOptions ro;
	int& width{ro.width};
	int& height{ro.height};

	AppOptions ao;

	using accu_scalar = unsigned int;
	std::vector<unsigned char> texture;
	std::vector<accu_scalar> accu;

	Shader line_shader;
	Fbo fbo;

	trace_options<T> to;
	lines<T> lines;

	app(RenderOptions ro, AppOptions ao)
	: ro(ro)
	, ao(ao)
	, texture(width * height, 0)
	, accu(width * height, 0)
	, line_shader(load_shader("resources/line", {"position", "color"}))
	, fbo(width, height)
	{
		const float margin = 20;
		to.bounds.xmin = -margin;
		to.bounds.ymin = -margin;
		to.bounds.xmax = width + margin;
		to.bounds.ymax = height + margin;

		float line_length = ao.line_length;
		for(int i = 0; i < ao.number_of_lines; ++i){
			float x = rand() / float(RAND_MAX) * width;
			float y = rand() / float(RAND_MAX) * height;

			float x2 = rand() / float(RAND_MAX) * line_length - 0.5f * line_length;
			float y2 = rand() / float(RAND_MAX) * line_length - 0.5f * line_length;

			lines.emplace_back(Vec2{x, y}, Vec2{x2, y2});
		}

		auto projectionMatrix = moggle::projection_matrices::orthographic(0, width, height, 0, -100.0f, 100.0f);
		auto modelViewProjectionMatrix = projectionMatrix;

		line_shader.s.use();
		line_shader.s.uniform<moggle::matrix<float, 4>>("modelViewProjectionMatrix").set(modelViewProjectionMatrix);
		
		fbo.bind();
		fbo.set_viewport();

		moggle::gl::enable(GL_BLEND);
		moggle::gl::blend_function(GL_ONE, GL_ONE);
		// moggle::gl::enable(GL_LINE_SMOOTH);
		// glLineWidth(0.5f);
		clear_fbo();
	}

	void clear_fbo(){
		moggle::gl::clear_color(0.0, 0.0, 0.0, 0.0);
		moggle::gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	std::vector<LineVertex> produce(Vec2 starting_position){
		auto tr = trace(to, {starting_position, Vec2{0, 0}}, lines);

		std::vector<LineVertex> output;
		output.reserve(ao.line_bound);

		T local_t = 0;
		auto it = tr.begin();
		auto next = it + 1;
		for(T t = 0; t < to.max_time; t += ao.dt){
			if(next != tr.end() && t > next->time){
				it = next++;
				output.emplace_back(it->position, ro.color);
				local_t = t - it->time;
			}
			Vec2 position = T{0.5}*to.gravity*local_t*local_t + it->velocity*local_t + it->position;
			output.emplace_back(position, ro.color);

			local_t += ao.dt;
		}

		return output;
	}

	void consume(std::vector<LineVertex> line_vertices){
		moggle::gl::enable_vertex_attribute_array(line_shader.attribute_location("position"));
		moggle::gl::vertex_attribute_pointer(line_shader.attribute_location("position"), 2, GL_DOUBLE, GL_FALSE, sizeof(LineVertex), &line_vertices[0].position);
		moggle::gl::enable_vertex_attribute_array(line_shader.attribute_location("color"));
		moggle::gl::vertex_attribute_pointer(line_shader.attribute_location("color"), 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LineVertex), &line_vertices[0].color);

		moggle::gl::draw_arrays(GL_LINE_STRIP, 0, line_vertices.size());
	}

	void download_fbo(){
		// We assume width/height are ok. This call is expensive
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, texture.data());
	}

	void accumulate_fbo(){
		for(auto i = 0; i < texture.size(); ++i){
			accu[i] += texture[i];
		}
	}

	void finalize(std::string filename){
		const double max = *std::max_element(accu.begin(), accu.end());
		const double sum = std::accumulate(accu.begin(), accu.end(), 0u);
		const double average = sum / accu.size();
		const double power = -ao.gamma/std::log(average/max);

		png::gray_ostream image(width, height, filename);
		for(auto x : accu){
			image << ao.exposure * std::pow(x / max, power);
		}
	}
};