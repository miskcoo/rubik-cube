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

namespace rubik_cube
{

namespace __krof_algo_impl
{

class krof_t : public algo_t
{
public:
	krof_t();
	~krof_t();
public:
	void init(const char*);
	void save(const char*) const;
	move_seq_t solve(cube_t) const;
private:
	int estimate(const cube_t&) const;

	typedef std::pair<int64_t, int32_t> cube_encode_t;

	struct cube_encode_hash_t
	{
		std::size_t operator() (const cube_encode_t& c) const
		{
			std::size_t h1 = std::hash<uint64_t>{}(c.first);
			std::size_t h2 = std::hash<uint32_t>{}(c.second);
			return h1 ^ (h2 << 1);
		}
	};

	template<int, int>
	static int encode_perm(const int *perm, const int *k);
	static int encode_corners(const cube_t&);
	static int encode_edges1(const cube_t&);
	static int encode_edges2(const cube_t&);
	static cube_encode_t encode_cube(const cube_t&);

	void init0(int8_t *buf, int(*encoder)(const cube_t&));
private:
	static const int disallow_faces[6];
	static const int edges_color_map[2][6];
	static const int corners_size = 88179840; // 3^7 * 8!
	static const int edges_size = 42577920;   // 2^6 * 12! / 6!
	int8_t corners[corners_size];
	int8_t edges1[edges_size];
	int8_t edges2[edges_size];

}; // class krof_t


const int krof_t::disallow_faces[6] = { -1, -1, -1, 1, 2, 0 };
const int krof_t::edges_color_map[][6] = { { 0, 1, 1, 1, 1, 0 }, { 0, 1, 0, 1, 0, 0 } };

krof_t::krof_t()
{
}

krof_t::~krof_t()
{
}

move_seq_t krof_t::solve(cube_t cb) const
{
	struct krof_search_data_t
	{
		uint8_t g, h;
		int64_t prev;
	};

	auto encode_move = [](int face, int cnt) -> uint8_t {
		return face << 4 | cnt;
	};

	auto decode_move = [](uint8_t move) -> move_step_t {
		return { face_t::face_type(move >> 4), move & 0xf };
	};

	cube_encode_t final_state = encode_cube({});

	for(int depth = 0; ; ++depth)
	{
//		printf("Solving %d...\n", depth);
		std::queue<krof_search_data_t> H;
		std::vector<std::pair<int64_t, uint8_t>> P;
		std::unordered_set<cube_encode_t, cube_encode_hash_t> M;
		H.push( { 0, (uint8_t)estimate(cb), -1 } );
		P.push_back( { -1, encode_move(6, 0) } );

		std::vector<uint8_t> temp_seq(depth);

		for(int64_t id = 0; !H.empty(); ++id)
		{
			krof_search_data_t s = H.front();
			int face = P[id].second >> 4;

			// calc current cube state
			int cnt = 0;
			for(auto p = P[id]; p.first != -1; p = P[p.first])
				temp_seq[cnt++] = p.second;

			cube_t cube0 = cb;
			for(int i = cnt - 1; i >= 0; --i)
			{
				move_step_t move = decode_move(temp_seq[i]);
				cube0.rotate(move.first, move.second);
			}

			for(int i = 0; i != 6; ++i)
			{
				if(i == face || disallow_faces[i] == face)
					continue;

				cube_t cube = cube0;
				for(int j = 1; j <= 3; ++j)
				{
					cube.rotate(face_t::face_type(i), 1);
					int h = estimate(cube);
					if(h + s.g + 1 <= depth)
					{
						cube_encode_t state = encode_cube(cube);
						if(state == final_state)
						{
							move_seq_t seq;
							seq.push_back( { face_t::face_type(i), j } );

							for(auto p = P[id]; p.first != -1; p = P[p.first])
								seq.push_back(decode_move(p.second));

							for(auto& r : seq)
								if(r.second == 3)
									r.second = -1;

							std::reverse(seq.begin(), seq.end());

							return seq;
						}

						if(M.count(state) == 0)
						{
							M.insert(state);
							H.push( { uint8_t(s.g + 1), uint8_t(h), id } );
							P.push_back( { id, encode_move(i, j) } );
						}
					}
				}
			}

			H.pop();
		}
	}

	return {};
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

krof_t::cube_encode_t krof_t::encode_cube(const cube_t& c)
{
	static const int k[12] = 
	{ 
		1, 12, 132, 1320, 11880, 
		95040, 665280, 3991680, 
		19958400, 79833600, 
		239500800, 479001600
	};

	edge_block_t eb = c.getEdgeBlock();

	int v = 0;
	for(int i = 0; i != 12; ++i)
	{
		int t = (eb.permutation[i] -= 8);
		v |= edges_color_map[7 >= t && t >= 4][eb.color[i]] << t;
	}

	return {
		v + ((long long)encode_perm<12, 11>(eb.permutation, k) << 12),
		encode_corners(c)
	};

}

} // namespace __krof_algo_impl

std::shared_ptr<algo_t> create_krof_algo()
{
	return std::make_shared<__krof_algo_impl::krof_t>();
}

} // namespace rubik_cube
