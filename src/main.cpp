#include <chrono>
#include <iostream>

#include "opengl.hpp"
#include "app.hpp"
#include "renderoptions.hpp"

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
		for(int i = 0; i < ro.chunks; ++i){
			a.clear_fbo();
			for(int j = 0; j < ro.chunk_size; ++j){
				float x = double(ro.width) * (double(i) / double(ro.samples) + double(j) / double(ro.chunk_size));
				float y = 0.0f;
				a.draw({x, y});
			}
			a.download_fbo();
			a.accumulate_fbo();
		}
	}

	{	Timer timer("exporting");
		a.finalize(filename);
	}
}

int main(int argc, char const *argv[]){
	context c;

	for(auto i : {0, 1, 2}){
		srand(i);
		auto ro = RenderOptions::defaults(1280, 800, 2);

		AppOptions ao;
		ao.number_of_lines = 100;

		std::cout << std::endl;
		std::string filename = "out_" + std::to_string(i) + ".png";
		render(ro, ao, filename);
		std::cout << "finished " << filename << std::endl;
	}
}
