#pragma once

#include <moggle/core/shader.hpp>

// Wrapper to use it non-vao things, because we need to know where the attributes are
struct Shader{
	moggle::shader_program s;
	std::vector<std::string> a;

	GLuint attribute_location(std::string attribute) const {
		GLuint c = 0;
		for(auto str : a){
			if(str == attribute) break;
			else ++c;
		}
		return c;
	}
};

Shader load_shader(std::string file, std::vector<std::string> attributes){
	Shader shader;
	shader.a = attributes;

	try {
		auto v = moggle::shader::from_file(moggle::shader_type::vertex, file + ".vsh");
		auto f = moggle::shader::from_file(moggle::shader_type::fragment, file + ".fsh");

		shader.s.attach(v);
		shader.s.attach(f);
	} catch (std::exception & e){
		std::cerr << e.what();
	}

	for(auto str : attributes){
		shader.s.bind_attribute(shader.attribute_location(str), str);
	}

	shader.s.link();
	return shader;
}
