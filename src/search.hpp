#ifndef __SEARCH_HPP__
#define __SEARCH_HPP__

#include "cube.h"
#include "algo.h"
#include "search.hpp"
#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace rubik_cube
{

struct search_info_t
{
	cube_t cb;
	int g, face, depth;

	move_seq_t* seq;

	int tid;
	std::atomic<int>* result_id;
};

template<typename SearchFunc>
inline bool search_multi_thread(
	int thread_num, 
	const search_info_t& s,
	SearchFunc search)
{
	search_info_t infos[18];
	move_seq_t seqs[18];

	std::mutex cv_m;
	std::condition_variable cv;
	int working_thread = 0;

	std::future<bool> results[18];
	std::atomic<int> result_id;
	result_id = -1;

	for(int i = 0; i != 6; ++i)
	{
		cube_t cube = s.cb;
		for(int j = 1; j <= 3; ++j)
		{
			int id = i * 3 + j - 1;
			cube.rotate(face_t::face_type(i), 1);

			seqs[id].resize(s.depth);
			seqs[id][0] = move_step_t{face_t::face_type(i), j};

			infos[id]           = s;
			infos[id].tid       = id;
			infos[id].cb        = cube;
			infos[id].seq       = seqs + id;
			infos[id].g         = 1;
			infos[id].face      = i;
			infos[id].result_id = &result_id;

			std::packaged_task<bool()> task {
				[&, id] () -> bool {

					std::unique_lock<std::mutex> lk(cv_m);
					cv.wait(lk, [&] { 
						return working_thread < thread_num; 
					} );

					++working_thread;
					lk.unlock();

					bool ret = search(infos[id]);

					{
						std::lock_guard<std::mutex> lk(cv_m);
						--working_thread;
					}

					cv.notify_one();

					return ret;
				}
			};

			results[id] = task.get_future();

			std::thread{ std::move(task) }.detach();
		}
	}

	cv.notify_all();

	for(auto& fu : results)
		fu.wait();

	if(result_id >= 0)
	{
		*s.seq = seqs[result_id];
		return true;
	} else return false;
}
} // namespace rubik_cube

#endif // __SEARCH_HPP__
