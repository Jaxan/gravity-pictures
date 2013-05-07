#pragma once

#include <algorithm>
#include <cassert>

struct RenderOptions{
	int width{}, height{};
	int samples{};
	int chunk_size{};
	int chunks{};
	unsigned char color;

	static RenderOptions defaults(int width = 1280, int height = 800, int multi_sampling = 8, int chunk_size = 512){
		RenderOptions ro;
		ro.width = width;
		ro.height = height;
		ro.samples = multi_sampling*width;
		ro.chunk_size = chunk_size;
		ro.chunks = ro.samples / ro.chunk_size;

		// samples should be divisible by chunk_size
		assert(ro.samples % ro.chunk_size == 0);

		ro.color = std::min(255, std::max(1, 256/ro.chunk_size));

		return ro;
	}
};