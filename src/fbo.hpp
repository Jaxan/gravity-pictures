//
//  fbo.h
//  J
//
//  Created by Joshua Moerman on 7/01/11.
//  Copyright 2011 Vadovas. All rights reserved.
//

/*
 Like the shader object, all work is done in the constructor and destructor. So copying a fbo object is impossible, use smart pointers instead. The standard constructor only takes a width and height. Texture settings and the presence of a depth buffer are preset and cannot be changed (since one probably wants those things, and there is not a lot of flexibility in ES2).

 The other constructor (wich is defined in fbo.mm) is to make EAGLView more compact, it makes a fbo in a given context and with a given layer (no texture will be generated, so you cannot use this fbo for texture lookup).

 Just like the shader, this class has begin() and end() functions.
 */

#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>

#include <moggle/core/gl.hpp>

namespace moggle { namespace gl {
inline bool check_current_framebuffer_status(){
	GLenum status = check_framebuffer_status(GL_FRAMEBUFFER);
	return (status == GL_FRAMEBUFFER_COMPLETE);
}
inline void print_current_framebuffer_status(){
	// check FBO status
	GLenum status = check_framebuffer_status(GL_FRAMEBUFFER);
	switch(status)
	{
		case GL_FRAMEBUFFER_COMPLETE:
			//COUT << "complete" << std::std::endl;
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cerr << "FBO: Attachment is NOT complete" << std::endl;
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cerr << "FBO: Missing attachment" << std::endl;
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			std::cerr << "FBO: Incomplete draw buffer" << std::endl;
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			std::cerr << "FBO: Incomplete read buffer" << std::endl;
			break;

		#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
			std::cerr << "FBO: Attached buffers have different dimensions" << std::endl;
			break;
		#endif

		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cerr << "FBO: Unsupported by implementation" << std::endl;
			break;

		default:
			std::cerr << "FBO: Unknown error: " << status << std::endl;
			break;
	}
}
}}

// TODO: more error checking in debug build.
// TODO: make texture class? for easier switching between linear/nearest interpolation for example. (also affects shader::set_texture).
// NOTE: stencil attachment is not supported
// NOTE: We use a renderbuffer for depth, we could also use a texture there if we would like to read the depth buffer.

class Renderbuffer {
	GLuint id = 0;

public:
	Renderbuffer(){
		moggle::gl::generate_renderbuffers(1, &id);
	}

	Renderbuffer(Renderbuffer const&) = delete;
	Renderbuffer& operator=(Renderbuffer const&) = delete;

	Renderbuffer(Renderbuffer && other){
		*this = std::move(other);
	}

	Renderbuffer& operator=(Renderbuffer && other){
		std::swap(id, other.id);
		return *this;
	}

	~Renderbuffer(){
		if(id != 0){
			moggle::gl::delete_renderbuffers(1, &id);
		}
	}

	void bind(GLenum const target) const {
		moggle::gl::bind_renderbuffer(target, id);
	}

	void storage(GLenum const target, GLenum format, GLsizei width, GLsizei height){
		bind(target);
		moggle::gl::renderbuffer_storage(target, format, width, height);
	}

	GLuint get_id() const {
		return id;
	}
};

class Fbo {
    std::vector<Renderbuffer> renderbuffers{};

	int width = 0, height = 0;
	GLuint id = 0;
	GLuint texture_id = 0;

public:
	Fbo(Fbo const&) = delete;
	Fbo& operator=(Fbo const&) = delete;

	Fbo(Fbo && other){
		*this = std::move(other);
	}

	Fbo& operator=(Fbo && other){
		std::swap(renderbuffers, other.renderbuffers);
		std::swap(width, other.width);
		std::swap(height, other.height);
		std::swap(id, other.id);
		std::swap(texture_id, other.texture_id);

		return *this;
	}

    // standard ctor, makes use of textures
    // Uses argument type deduction
	Fbo(int width_, int height_)
	: width(width_)
	, height(height_)
	{
		moggle::gl::generate_framebuffers(1, &id);
		bind();

		// generate texture
		moggle::gl::generate_textures(1, &texture_id);
		moggle::gl::bind_texture(GL_TEXTURE_2D, texture_id);
		moggle::gl::texture_image_2d(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		moggle::gl::framebuffer_texture_2d(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

		// attach depth renderbuffer
		// renderbuffers.emplace_back();
		// Renderbuffer& b = renderbuffers.back();
		// b.storage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
		// attach(b, GL_DEPTH_ATTACHMENT);

		if(!moggle::gl::check_current_framebuffer_status()){
			std::cerr << "Framebuffer could not be created:\n";
			moggle::gl::print_current_framebuffer_status();
			throw std::runtime_error("Framebuffer not created.");
		}

		moggle::gl::clear_color(0.0, 0.0, 0.0, 0.0);
		moggle::gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		unbind();
	}

	~Fbo() {
		moggle::gl::delete_framebuffers(1, &id);
	        moggle::gl::delete_textures(1, &texture_id);
	}

	//! Sets the viewport to the size of this FBO
	void set_viewport() const {
		moggle::gl::viewport(0, 0, width, height);
	}

	void bind(){
		moggle::gl::bind_framebuffer(GL_FRAMEBUFFER, id);
	}

	void unbind(){
		moggle::gl::bind_framebuffer(GL_FRAMEBUFFER, 0);
	}

	void attach(Renderbuffer const& b, GLenum attachment_point){
		moggle::gl::framebuffer_renderbuffer(GL_FRAMEBUFFER, attachment_point, GL_RENDERBUFFER, b.get_id());
	}

	GLuint get_texture_id() const {
		return texture_id;
	}

	int get_width() const {
		return width;
	}

	int get_height() const {
		return height;
	}
};
