#include "viewer.h"
#include "algo.h"
#include <random>
#include <cstdio>
#include <fstream>
using namespace rubik_cube;

std::random_device rd;
std::mt19937 mt(rd());
const char* data_file = "krof.dat";
const char* face_str = "UDFBLR";

bool is_file_exist(const char* filename)
{
	std::ifstream ifs(filename);
	return ifs.good();
}

void put_rotate(int c1, int c2)
{
	std::putchar(face_str[c1]);
	if(c2 > 1) std::putchar(c2 == 2 ? '2' : '\'');
}

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		std::puts("Usage: ./krof thread_num random_rotate_num");
		return 0;
	}

	auto viewer = create_opengl_viewer();

	int thread_num = std::atoi(argv[1]);

	auto algo = create_krof_algo(thread_num);

	if(is_file_exist(data_file))
	{
		std::puts("Reading data file...");
		algo->init(data_file);
	} else {
		std::puts("Initializing heuristic function table...");
		algo->init();
		algo->save("krof.dat");  
	}

	cube_t c;

	std::uniform_int_distribution<int> gen(0, 5);
	std::uniform_int_distribution<int> gen2(1, 3);

	move_seq_t ans;

	std::puts("Generating cube...");
	c = cube_t();
	for(int i = 0; i != std::atoi(argv[2]); ++i)
	{
		int c1 = gen(mt), c2 = gen2(mt);
		c.rotate(face_t::face_type(c1), c2); 
		put_rotate(c1, c2);
	}

	std::puts("\nCalculating optimal solution...");
	ans = algo->solve(c);
	std::puts("");

	viewer->init(argc, argv);
	viewer->set_rotate_duration(1.0);
	viewer->set_cube(c);
	for(auto x : ans)
	{
		put_rotate(int(x.first), (x.second % 4 + 4) & 3);
		viewer->add_rotate(x.first, x.second);
	}

	std::puts("\nFinished!");
	std::fflush(stdout);
	viewer->run();
	return 0;
}
