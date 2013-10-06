#include <chrono>
#include <iostream>
#include <thread>

#include <lock_queue.hpp>
#include <thread_pool.hpp>

#include "opengl.hpp"
#include "renderoptions.hpp"
#include "app.hpp"

// RAII timer
struct Timer {
	using clock = std::chrono::high_resolution_clock;
	using time_point = std::chrono::time_point<clock>;
	time_point start = clock::now();
	std::string name;

	inline Timer(std::string name_) : name(name_) {}

	inline ~Timer(){
		auto const end = clock::now();
		std::chrono::duration<double, std::milli> duration = end - start;
		std::cout << name << " took: " << duration.count() << " ms" << std::endl;
	}
};

void render(RenderOptions ro, AppOptions ao, std::string filename){
	Timer timer("total time");
	app a(ro, ao);

	{	Timer timer("rendering");

		queue<std::vector<LineVertex>> buffers;
		thread_pool tp;

		for(int i = 0; i < ro.chunks; ++i){
			for(int j = 0; j < ro.chunk_size; ++j){
				tp.add([&a, &ro, &buffers, i, j]{
					float x = double(ro.width) * (double(i) / double(ro.samples) + double(j) / double(ro.chunk_size));
					float y = 800.0f;
					buffers.push(a.produce({x, y}));
				});
			}
		}

		int nthreads = std::thread::hardware_concurrency() - 1;
		if(nthreads < 1) nthreads = 1;
		tp.run(nthreads);

		for(int i = 0; i < ro.chunks; ++i){
			for(int j = 0; j < ro.chunk_size;){
				if(auto b = buffers.try_pop()) {
					a.consume(*b);
					++j;
				} else {
					std::this_thread::yield();
				}
			}
			a.download_fbo();
			a.accumulate_fbo();
			a.clear_fbo();
		}
	}

	{	Timer timer("exporting");
		a.finalize(filename);
	}
}

int main(int argc, char const *argv[]){
	context c;

	for(auto i : {1, 2, 3}){
		srand(i);
		auto ro = RenderOptions::defaults(1280, 800, 8);

		AppOptions ao;

		std::cout << std::endl;
		std::string filename = "new_" + std::to_string(i) + ".png";
		render(ro, ao, filename);
		std::cout << "finished " << filename << std::endl;
	}
}
