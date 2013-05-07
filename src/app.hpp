#pragma once

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>

#include <beats-sim/Simulation.h>
#include <ImageStreams/include/png.hpp>
#include <moggle/math/projection.hpp>

#include "opengl.hpp"
#include "shader.hpp"
#include "fbo.hpp"
#include "renderoptions.hpp"

using namespace simulation;
using namespace math;

using ball_info = void;
using line_info = void;
using simu_type = Simulation<ball_info, line_info>;

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
	float dt{1.0f / 240.0f};
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

	simu_type sim;

	app(RenderOptions ro, AppOptions ao)
	: ro(ro)
	, ao(ao)
	, texture(width * height, 0)
	, accu(width * height, 0)
	, line_shader(load_shader("resources/line", {"position", "color"}))
	, fbo(width, height)
	{
		const float margin = 20;
		sim.bounds.xmin = -margin;
		sim.bounds.ymin = -margin;
		sim.bounds.xmax = width + margin;
		sim.bounds.ymax = height + margin;

		float line_length = ao.line_length;
		for(int i = 0; i < ao.number_of_lines; ++i){
			float x = rand() / float(RAND_MAX) * width;
			float y = rand() / float(RAND_MAX) * height;

			float x2 = x + rand() / float(RAND_MAX) * line_length - 0.5f * line_length;
			float y2 = y + rand() / float(RAND_MAX) * line_length - 0.5f * line_length;

			sim.lines.emplace_back(Vec2{x, y}, Vec2{x2, y2}, kOneWay);
		}

		auto projectionMatrix = moggle::projection_matrices::orthographic(0, width, 0, height, -100.0f, 100.0f);
		auto modelViewProjectionMatrix = projectionMatrix;

		line_shader.s.use();
		line_shader.s.uniform<moggle::matrix<float, 4>>("modelViewProjectionMatrix").set(modelViewProjectionMatrix);
		
		fbo.bind();
		fbo.set_viewport();

		moggle::gl::enable(GL_BLEND);
		moggle::gl::blend_function(GL_ONE, GL_ONE);
		// moggle::gl::enable(GL_LINE_SMOOTH);
		// glLineWidth(0.5f);
	}

	void clear_fbo(){
		moggle::gl::clear_color(0.0, 0.0, 0.0, 0.0);
		moggle::gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void draw(Vec2 position){
		sim.balls.clear();
		sim.balls.emplace_back(position.x, position.y, 0.0, 0.0);

		{
			std::vector<LineVertex> lines;

			lines.reserve(ao.line_bound);
			int count = ao.line_bound;
			while(sim.balls.size() == 1 && count--){
				lines.emplace_back(sim.balls.front().position, ro.color);
				sim.update(ao.dt);
			}

			moggle::gl::enable_vertex_attribute_array(line_shader.attribute_location("position"));
			moggle::gl::vertex_attribute_pointer(line_shader.attribute_location("position"), 2, GL_FLOAT, GL_FALSE, sizeof(LineVertex), &lines[0].position.x);
			moggle::gl::enable_vertex_attribute_array(line_shader.attribute_location("color"));
			moggle::gl::vertex_attribute_pointer(line_shader.attribute_location("color"), 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LineVertex), &lines[0].color);

			moggle::gl::draw_arrays(GL_LINE_STRIP, 0, lines.size());
		}
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