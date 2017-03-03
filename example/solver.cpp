#include "viewer.h"
#include "algo.h"
#include <random>
#include <cstdio>
#include <fstream>
#include <map>
#include <string>
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

void output_usage(std::string str = "")
{
	if(!str.empty())
		std::printf("Error: %s\n", str.c_str());
	std::puts("Usage: ./solver -tra");
	std::puts("-t maximum thread used to calculate [default: 1].");
	std::puts("-r random rotation times to generate a cube.");
	std::puts("   when using Krof, default is 15.");
	std::puts("   when using Krociemba, default is 200.");
	std::puts("-a which algorithm to be used to solve the cube.");
	std::puts("   only two algorithms available: krof, kociemba.");
	std::puts("   [default: kociemba]");
	std::exit(0);
}

int main(int argc, char** argv)
{
	std::map<std::string, std::string> M;
	for(int i = 1; i != argc; ++i)
	{
		if(argv[i][0] == '-')
		{
			std::string key = argv[i] + 1;
			if(key != "t" && key != "r" && key != "a")
				output_usage();

			M[key] = argv[++i];
		}
	}

	std::string algo_type = "kociemba";

	if(M.count("a"))
	{
		if(M["a"] != "kociemba" && M["a"] != "krof")
			output_usage();
		algo_type = M["a"];
	}

	int random_times = algo_type == "krof" ? 15 : 200;

	if(M.count("r"))
	{
		random_times = std::atoi(M["r"].c_str());
		if(random_times < 0)
			output_usage("random times cannot be negative!");
	}

	int thread_num = 4;
	if(M.count("t"))
	{
		thread_num = std::atoi(M["t"].c_str());
		if(thread_num < 0)
			output_usage("thread number cannot be negative!");
		else if(thread_num > 32)
			output_usage("thread number is too large!");
	}

	auto viewer = create_opengl_viewer();

	std::shared_ptr<algo_t> algo;
	if(algo_type == "krof")
	{
		algo = create_krof_algo(thread_num);

		if(is_file_exist(data_file))
		{
			std::puts("Reading data file...");
			algo->init(data_file);
		} else {
			std::puts("Initializing heuristic function table...");
			algo->init();
			algo->save("krof.dat");  
		}
	} else {
		algo = create_kociemba_algo(thread_num);

		std::puts("Initializing heuristic function table...");
		algo->init();
	}

	cube_t c;

	std::uniform_int_distribution<int> gen(0, 5);
	std::uniform_int_distribution<int> gen2(1, 3);

	move_seq_t ans;

	std::puts("Generating cube...");
	c = cube_t();
	for(int i = 0; i != random_times; ++i)
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

	std::printf("\nSolution needs %d steps.", (int)ans.size());
	std::fflush(stdout);
	viewer->run();
	return 0;
}
