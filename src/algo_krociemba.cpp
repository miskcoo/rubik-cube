#include "algo.h"
#include "cube.h"
#include "search.hpp"
#include <cstring>

namespace rubik_cube
{

namespace __krociemba_algo_impl
{

class krociemba_t : public algo_t
{
public:
	krociemba_t() = default;
	~krociemba_t() = default;
public:
	void init(const char*);
	void save(const char*) const;
	move_seq_t solve(cube_t) const;
private:
	static int encode_phrase1_edges(const cube_t&);
	static int encode_phrase1_co(const cube_t&);
	static int encode_phrase1_eo(const cube_t&);
	static int encode_phrase2_corners(const cube_t&);
	static int encode_phrase2_edges1(const cube_t&);
	static int encode_phrase2_edges2(const cube_t&);
private:
	template<int Size, typename PushFunc>
	static void init_permutation(int, int8_t*, const PushFunc&, int);
private:
	struct search_info_t
	{
		cube_t cb;
		int g, face, depth;

		move_seq_t* seq;
	};

	template<int Phrase>
	bool search_phrase(const search_info_t&) const;
	template<int Phrase>
	int estimate(const cube_t&) const;

	int estimate_phrase1(const cube_t&) const;
	int estimate_phrase2(const cube_t&) const;
private:
	static const int phrase2_corners_size = 40320; // 8!
	static const int phrase2_edges1_size = 40320;  // 8!
	static const int phrase2_edges2_size = 24;     // 4!
	static const int phrase1_edges_size = 12 * 11 * 10 * 9 * 16;
	static const int phrase1_co_size = 6561;
	static const int phrase1_eo_size = 1 << 8;
	int8_t phrase2_corners[phrase2_corners_size];
	int8_t phrase2_edges1[phrase2_edges1_size];
	int8_t phrase2_edges2[phrase2_edges2_size];
	int8_t phrase1_edges[phrase1_edges_size];
	int8_t phrase1_co[phrase1_co_size];
	int8_t phrase1_eo[phrase1_eo_size];
}; // class krociemba_t

template<int Size, typename PushFunc>
void krociemba_t::init_permutation(int now, int8_t* v, const PushFunc& push, int avail)
{
	if(!avail) return push();

	for(int i = 0; i != Size; ++i)
	{
		if((avail >> i) & 1)
		{
			avail ^= 1 << i;
			v[now] = i;
			init_permutation<Size>(now + 1, v, push, avail);
			avail ^= 1 << i;
		}
	}
}

void krociemba_t::init(const char*)
{
	std::vector<cube_t> states;

	std::memset(phrase2_corners, 0xff, sizeof(phrase2_corners));
	init_heuristic<true>(phrase2_corners, &krociemba_t::encode_phrase2_corners);

	std::memset(phrase2_edges1, 0xff, sizeof(phrase2_edges1));
	init_heuristic<true>(phrase2_edges1, &krociemba_t::encode_phrase2_edges1);

	std::memset(phrase2_edges2, 0xff, sizeof(phrase2_edges2));
	init_heuristic<true, true>(phrase2_edges2, &krociemba_t::encode_phrase2_edges2, &states);

	for(cube_t& c : states)
	{
		// fix the orientation
		int8_t *eo = const_cast<int8_t*>(c.getEdgeBlock().second);
		eo[0] = eo[1] = eo[2] = eo[3] = 0;
	}

	std::memset(phrase1_edges, 0xff, sizeof(phrase1_edges));
	init_heuristic<false>(phrase1_edges, &krociemba_t::encode_phrase1_edges, nullptr, states);

	states.clear();

	cube_t cube0;
	int8_t *cube0_ep = const_cast<int8_t*>(cube0.getCornerBlock().first);
	init_permutation<8>(0, cube0_ep, [&]() {
		states.push_back(cube0);
	}, (1 << 8) - 1 );

	std::memset(phrase1_co, 0xff, sizeof(phrase1_co));
	init_heuristic<false>(phrase1_co, &krociemba_t::encode_phrase1_co, nullptr, states);

	states.clear();
	cube0 = cube_t();

	init_permutation<12>(4, cube0_ep, [&]() {
		states.push_back(cube0);
	}, ((1 << 8) - 1) << 4 );

	std::memset(phrase1_eo, 0xff, sizeof(phrase1_eo));
	init_heuristic<false>(phrase1_eo, &krociemba_t::encode_phrase1_eo, nullptr, states);
}

void krociemba_t::save(const char*) const
{
	// do nothing
}

move_seq_t krociemba_t::solve(cube_t cb) const
{
	move_seq_t solution;

	// phrase 1
	for(int depth = 0; ; ++depth)
	{
		move_seq_t seq(depth);

		search_info_t s;
		s.cb    = cb;
		s.g     = 0;
		s.seq   = &seq;
		s.face  = 6;
		s.depth = depth;

		if(search_phrase<1>(s))
		{
			solution = seq;
			break;
		}
	}

	for(move_step_t& step : solution)
		cb.rotate(step.first, step.second);

	// phrase 2
	for(int depth = 0; ; ++depth)
	{
		move_seq_t seq(depth);

		search_info_t s;
		s.cb    = cb;
		s.g     = 0;
		s.seq   = &seq;
		s.face  = 6;
		s.depth = depth;

		if(search_phrase<2>(s))
		{
			if(seq.front().first == solution.back().first)
			{
				// merge rotation of same faces;
				seq[0].second = (seq[0].second + solution.back().second) % 4;
				solution.pop_back();
			}  

			for(move_step_t step : seq)
				solution.push_back(step);
			break;
		}
	}

	for(move_step_t& step : solution)
		if(step.second == 3)
			step.second = -1;

	return solution;
}

template<int Phrase>
bool krociemba_t::search_phrase(const search_info_t& s) const
{
#ifdef DEBUG
	static uint64_t cnt = 0;
	if(++cnt % 10000 == 0)
	{
		std::printf("\rdepth = % 3d, node = % 12ld ", s.depth, cnt);
		std::fflush(stdout);
	}
#endif

	search_info_t t = s;
	t.g += 1;

	for(int i = 0; i != 6; ++i)
	{
		if(i == s.face || disallow_faces[i] == s.face)
			continue;

		cube_t cube = s.cb;
		for(int j = 1; j <= 3; ++j)
		{
			if(Phrase == 1)
			{
				cube.rotate(face_t::face_type(i), 1);
			} else {
				if(i >= 2 && j != 2) continue;
				cube.rotate(face_t::face_type(i), i < 2 ? 1 : j);
			}

			int h = estimate<Phrase>(cube);
			if(h + s.g + 1 <= s.depth)
			{
				(*s.seq)[s.g] = move_step_t{face_t::face_type(i), j};

				if(h == 0)
					return true;

				t.cb   = cube;
				t.face = i;

				if(search_phrase<Phrase>(t))
					return true;
			}
		}
	}

	return false;
}

template<int Phrase>
int krociemba_t::estimate(const cube_t& c) const
{
	if(Phrase == 1) 
		return estimate_phrase1(c);
	return estimate_phrase2(c);
}

int krociemba_t::estimate_phrase1(const cube_t& c) const
{
	return std::max(
		phrase1_edges[encode_phrase1_edges(c)],
		std::max(
			phrase1_eo[encode_phrase1_eo(c)],
			phrase1_co[encode_phrase1_co(c)]
		)
	);
}

int krociemba_t::estimate_phrase2(const cube_t& c) const
{
	return std::max(
		phrase2_corners[encode_phrase2_corners(c)],
		std::max(
			phrase2_edges1[encode_phrase2_edges1(c)],
			phrase2_edges2[encode_phrase2_edges2(c)]
		)
	);
}

int krociemba_t::encode_phrase2_corners(const cube_t& c) 
{
	block_info_t cb = c.getCornerBlock();
	return encode_perm<8, 7>(cb.first, factorial_8);
}

int krociemba_t::encode_phrase2_edges1(const cube_t& c) 
{
	block_info_t eb = c.getEdgeBlock();
	int8_t perm[8];
	for(int i = 4; i != 12; ++i)
		perm[i - 4] = eb.first[i] - 4;

	return encode_perm<8, 7>(perm, factorial_8);
}

int krociemba_t::encode_phrase2_edges2(const cube_t& c) 
{
	block_info_t eb = c.getEdgeBlock();
	return encode_perm<4, 3>(eb.first, factorial_4);
}

int krociemba_t::encode_phrase1_edges(const cube_t& c) 
{
	block_info_t eb = c.getEdgeBlock();
	int v = 0;
	int8_t perm[4];
	for(int i = 0; i != 12; ++i)
		if(eb.first[i] < 4)
		{
			perm[eb.first[i]] = i;
			v |= eb.second[i] << eb.first[i];
		}

	return v | (encode_perm<12, 4>(perm, factorial_12) << 4);
}

int krociemba_t::encode_phrase1_co(const cube_t& c)
{
	static const int base[] = { 1, 3, 9, 27, 81, 243, 729, 2187 };
	block_info_t cb = c.getCornerBlock();
	int v = 0;
	for(int i = 0; i != 8; ++i)
		v += cb.second[i] * base[i];
	return v;
}

int krociemba_t::encode_phrase1_eo(const cube_t& c)
{
	block_info_t eb = c.getEdgeBlock();
	int v = 0;
	for(int i = 0; i != 8; ++i)
		v |= eb.second[i + 4] << i;
	return v;
}

} // namespace __krociemba_algo_impl

std::shared_ptr<algo_t> create_krociemba_algo()
{
	return std::make_shared<__krociemba_algo_impl::krociemba_t>();
}

} // namespace rubik_cube
