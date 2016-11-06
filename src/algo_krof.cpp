#include "algo.h"
#include "cube.h"
#include <tuple>
#include <queue>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <unordered_set>

#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace rubik_cube
{

namespace __krof_algo_impl
{

class krof_t : public algo_t
{
public:
	krof_t(int thread_num);
	~krof_t();
public:
	void init(const char*);
	void save(const char*) const;
	move_seq_t solve(cube_t) const;
private:
	template<int, int>
	static int encode_perm(const int *perm, const int *k);
	static int encode_corners(const cube_t&);
	static int encode_edges1(const cube_t&);
	static int encode_edges2(const cube_t&);

	void init0(int8_t *buf, int(*encoder)(const cube_t&));
private:
	struct search_info_t
	{
		cube_t cb;
		int g, face, depth;

		move_seq_t* seq;

		int tid;
		std::atomic<int>* result_id;
	};

	int estimate(const cube_t&) const;
	bool search(search_info_t) const;
	bool search_multi_thread(search_info_t) const;
private:
	static const int disallow_faces[6];
	static const int edges_color_map[2][6];
	static const int corners_size = 88179840; // 3^7 * 8!
	static const int edges_size = 42577920;   // 2^6 * 12! / 6!
	int8_t corners[corners_size];
	int8_t edges1[edges_size];
	int8_t edges2[edges_size];
	int thread_num;
}; // class krof_t

const int krof_t::disallow_faces[6] = { -1, -1, -1, 1, 2, 0 };
const int krof_t::edges_color_map[][6] = { { 0, 1, 1, 1, 1, 0 }, { 0, 1, 0, 1, 0, 0 } };

krof_t::krof_t(int thread_num)
{
	this->thread_num = thread_num;
}

krof_t::~krof_t()
{
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
			if(search_multi_thread(s))
				return *s.seq;
		}
	}

	return {};
}

bool krof_t::search(search_info_t s) const
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

				search_info_t t = s;
				t.cb   = cube;
				t.face = i;
				t.g   += 1;

				if(search(t))
					return true;
			}
		}
	}

	return false;
}

bool krof_t::search_multi_thread(search_info_t s) const
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
						return working_thread < this->thread_num; 
					} );

					++working_thread;
					lk.unlock();

					bool ret = this->search(infos[id]);

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

int krof_t::estimate(const cube_t& c) const
{
	return std::max(
		corners[encode_corners(c)],
		std::max(
			edges1[encode_edges1(c)],
			edges2[encode_edges2(c)]
		)
	);
}

void krof_t::init(const char* filename)
{
	if(!filename)
	{
		std::memset(edges1, 0xff, sizeof(edges1));
		init0(edges1, &krof_t::encode_edges1);

		std::memset(edges2, 0xff, sizeof(edges2));
		init0(edges2, &krof_t::encode_edges2);

		std::memset(corners, 0xff, sizeof(corners));
		init0(corners, &krof_t::encode_corners);
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

void krof_t::init0(int8_t *buf, int(*encoder)(const cube_t&))
{
	std::queue<std::pair<cube_t, uint8_t>> que;
	buf[(*encoder)(cube_t())] = 0;
	que.push( { cube_t(), 0 | (6 << 4) } );

	while(!que.empty())
	{
		auto u = que.front();
		int face = u.second >> 4;
		int step = u.second & 0xf;

		for(int i = 0; i != 6; ++i)
		{
			if(i == face || disallow_faces[i] == face)
				continue;

			cube_t c = u.first;
			for(int j = 0; j != 3; ++j)
			{
				c.rotate(face_t::face_type(i), 1);
				int code = (*encoder)(c);
				if(buf[code] == -1)
				{
					buf[code] = step + 1;
					que.push( { c, (step + 1) | (i << 4) } );
				}
			}
		}

		que.pop();
	}
}

template<int N, int S>
int krof_t::encode_perm(const int *p, const int *k) 
{
	int pos[N], elem[N];

	for(int i = 0; i != N; ++i)
		pos[i] = elem[i] = i;

	int v = 0, t;
	for(int i = 0; i != S; ++i)
	{
		t = pos[p[i]];
		v += k[i] * t;
		pos[elem[N - i - 1]] = t;
		elem[t] = elem[N - i - 1];
	}

	return v;
}

int krof_t::encode_edges1(const cube_t& c)
{
	static const int k[6] = { 1, 12, 132, 1320, 11880, 95040 };

	edge_block_t eb = c.getEdgeBlock();

	int t, v = 0, perm[6];
	for(int i = 0; i != 12; ++i)
	{
		if(eb.permutation[i] < 14)
		{
			t = eb.permutation[i] - 8;
			perm[t] = i;
			v |= edges_color_map[t >= 4][eb.color[i]] << t;
		}
	}

	return v + (encode_perm<12, 6>(perm, k) << 6);
}

int krof_t::encode_edges2(const cube_t& c)
{
	static const int k[6] = { 1, 12, 132, 1320, 11880, 95040 };

	edge_block_t eb = c.getEdgeBlock();

	int t, v = 0, perm[6];
	for(int i = 0; i != 12; ++i)
	{
		if(eb.permutation[i] >= 14)
		{
			t = eb.permutation[i] - 14;
			perm[t] = i;
			v |= edges_color_map[t < 2][eb.color[i]] << t;
		}
	}

	return v + (encode_perm<12, 6>(perm, k) << 6);
}

int krof_t::encode_corners(const cube_t& c) 
{
	static const int base0 = 2187; // 3^7
	static const int color_map[6] = { 0, 1, 2, 1, 2, 0 };
	static const int k[7] = { 1, 8, 56, 336, 1680, 6720, 20160 };

	corner_block_t cb = c.getCornerBlock();
	int v = 0;
	for(int i = 0; i != 7; ++i)
		v = v * 3 + color_map[cb.top_bottom_color[i]];

	return v + encode_perm<8, 7>(cb.permutation, k) * base0;
}

} // namespace __krof_algo_impl

std::shared_ptr<algo_t> create_krof_algo(int thread_num)
{
	return std::make_shared<__krof_algo_impl::krof_t>(thread_num);
}

} // namespace rubik_cube
