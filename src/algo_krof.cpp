#include "algo.h"
#include "cube.h"
#include "search.hpp"
#include "heuristic.hpp"
#include <fstream>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <functional>

namespace rubik_cube
{

namespace __krof_algo_impl
{

class krof_t : public algo_t
{
public:
	krof_t(int thread_num);
	~krof_t() = default;
public:
	void init(const char*);
	void save(const char*) const;
	move_seq_t solve(cube_t) const;
private:
	static int encode_corners(const cube_t&);
	static int encode_edges1(const cube_t&);
	static int encode_edges2(const cube_t&);
private:
	int estimate(const cube_t&) const;
	int estimate_edges(const cube_t&) const;
	bool search(const search_info_t&) const;
private:
	static const int corners_size = 88179840; // 3^7 * 8!
	static const int edges_size = 42577920;   // 2^6 * 12! / 6!
	int8_t corners[corners_size];
	int8_t edges1[edges_size];
	int8_t edges2[edges_size];
	int thread_num;
}; // class krof_t


krof_t::krof_t(int thread_num)
{
	this->thread_num = thread_num;
}

move_seq_t krof_t::solve(cube_t cb) const
{
	for(int depth = 0; ; ++depth)
	{
		move_seq_t seq(depth);

		search_info_t s;
		s.cb    = cb;
		s.g     = 0;
		s.seq   = &seq;
		s.face  = 6;
		s.depth = depth;

		if(depth < 11 || thread_num == 1) 
		{
			s.tid = -1;
			if(search(s)) 
				return *s.seq;
		} else {
			using namespace std::placeholders;
			if(search_multi_thread(thread_num, s, std::bind(&krof_t::search, this, _1)))
				return *s.seq;
		}
	}

	return {};
}

bool krof_t::search(const search_info_t& s) const
{
#ifdef DEBUG
	static uint64_t cnt = 0;
	if(++cnt % 10000 == 0)
	{
		std::printf("\rdepth = % 3d, node = % 12ld ", s.depth, cnt);
		std::fflush(stdout);
	}
#endif

	if(s.tid >= 0 && *s.result_id >= 0)
		return true;

	search_info_t t = s;
	t.g += 1;

	for(int i = 0; i != 6; ++i)
	{
		if(i == s.face || disallow_faces[i] == s.face)
			continue;

		cube_t cube = s.cb;
		for(int j = 1; j <= 3; ++j)
		{
			cube.rotate(face_t::face_type(i), 1);
			int h = estimate(cube);
			if(h + s.g + 1 <= s.depth)
			{
				(*s.seq)[s.g] = move_step_t{face_t::face_type(i), j};

				if(h == 0)
				{
					for(auto& r : *s.seq)
						if(r.second == 3)
							r.second = -1;

					if(s.tid >= 0)
						*s.result_id = s.tid;

					return true;
				}

				t.cb   = cube;
				t.face = i;

				if(search(t))
					return true;
			}
		}
	}

	return false;
}

int krof_t::estimate(const cube_t& c) const
{
	return std::max<int>(
		corners[encode_corners(c)],
		estimate_edges(c)
	);
}

void krof_t::init(const char* filename)
{
	if(!filename)
	{
		std::memset(edges1, 0xff, sizeof(edges1));
		init_heuristic<false>(edges1, &krof_t::encode_edges1);

		std::memset(edges2, 0xff, sizeof(edges2));
		init_heuristic<false>(edges2, &krof_t::encode_edges2);

		std::memset(corners, 0xff, sizeof(corners));
		init_heuristic<false>(corners, &krof_t::encode_corners);
	} else {
		std::ifstream ifs(filename, std::ios::binary);
		ifs.read(reinterpret_cast<char*>(edges1), edges_size);
		ifs.read(reinterpret_cast<char*>(edges2), edges_size);
		ifs.read(reinterpret_cast<char*>(corners), corners_size);
	}
}

void krof_t::save(const char* filename) const
{
	std::ofstream ofs(filename, std::ios::binary);
	ofs.write(reinterpret_cast<const char*>(edges1), edges_size);
	ofs.write(reinterpret_cast<const char*>(edges2), edges_size);
	ofs.write(reinterpret_cast<const char*>(corners), corners_size);
}

int krof_t::estimate_edges(const cube_t& c) const
{
	block_info_t eb = c.getEdgeBlock();

	int t;
	int v1 = 0, v2 = 0;
	int8_t perm1[6], perm2[6];
	for(int i = 0; i != 12; ++i)
	{
		if(eb.first[i] < 6)
		{
			t = eb.first[i];
			perm1[t] = i;
			v1 |= eb.second[i] << t;
		} else {
			t = eb.first[i] - 6;
			perm2[t] = i;
			v2 |= eb.second[i] << t;
		}
	}

	return std::max(
		edges1[v1 + (encode_perm<12, 6>(perm1, factorial_12) << 6)],
		edges2[v2 + (encode_perm<12, 6>(perm2, factorial_12) << 6)]
	);
}

int krof_t::encode_edges1(const cube_t& c)
{
	block_info_t eb = c.getEdgeBlock();

	int t, v = 0;
	int8_t perm[6];
	for(int i = 0; i != 12; ++i)
	{
		if(eb.first[i] < 6)
		{
			t = eb.first[i];
			perm[t] = i;
			v |= eb.second[i] << t;
		}
	}

	return v + (encode_perm<12, 6>(perm, factorial_12) << 6);
}

int krof_t::encode_edges2(const cube_t& c)
{
	block_info_t eb = c.getEdgeBlock();

	int t, v = 0;
	int8_t perm[6];
	for(int i = 0; i != 12; ++i)
	{
		if(eb.first[i] >= 6)
		{
			t = eb.first[i] - 6;
			perm[t] = i;
			v |= eb.second[i] << t;
		}
	}

	return v + (encode_perm<12, 6>(perm, factorial_12) << 6);
}

int krof_t::encode_corners(const cube_t& c) 
{
	static const int base0 = 2187; // 3^7

	block_info_t cb = c.getCornerBlock();
	int v = 0;
	for(int i = 0; i != 7; ++i)
		v = v * 3 + cb.second[i];

	return v + encode_perm<8, 7>(cb.first, factorial_8) * base0;
}

} // namespace __krof_algo_impl

std::shared_ptr<algo_t> create_krof_algo(int thread_num)
{
	return std::make_shared<__krof_algo_impl::krof_t>(thread_num);
}

} // namespace rubik_cube
