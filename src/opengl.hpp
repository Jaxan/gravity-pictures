#pragma once

#include <stdexcept>
#include <iostream>

#include <OpenGL/OpenGL.h>

// Wrapper around the mac api calls
struct context {
	CGLContextObj ctx{};
	context(){
		CGLPixelFormatObj pix{};
		GLint npix{};
		const CGLPixelFormatAttribute attributes[]{kCGLPFAAllowOfflineRenderers, kCGLPFASupersample, static_cast<CGLPixelFormatAttribute>(0)};

		CGLError error = CGLChoosePixelFormat(attributes, &pix, &npix);
		if(error){
			std::cerr << "Could not choose pixel format\n" << CGLErrorString(error) << std::endl;
			throw std::runtime_error("Could not choose pixel format");
		}

		error = CGLCreateContext(pix, 0, &ctx);
		CGLDestroyPixelFormat(pix);
		if(error){
			std::cerr << "Could not create context\n" << CGLErrorString(error) << std::endl;
			throw std::runtime_error("Could not create context");
		}
		CGLSetCurrentContext(ctx);
	}

	~context(){
		CGLDestroyContext(ctx);
	}
};
