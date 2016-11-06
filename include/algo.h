#ifndef __ALGO_H__
#define __ALGO_H__
#include "cube.h"
#include <vector>
#include <memory>

namespace rubik_cube
{
	typedef std::pair<face_t::face_type, int> move_step_t;
	typedef std::vector<move_step_t> move_seq_t;

	class algo_t
	{
	public:
		virtual ~algo_t() = default;
	public:
		virtual void init(const char* filename = nullptr) = 0;
		virtual void save(const char* filename) const = 0;
		virtual move_seq_t solve(cube_t) const = 0;
	};

	std::shared_ptr<algo_t> create_krof_algo(int thread_num = 4);
}

#endif // __ALGO_H__
