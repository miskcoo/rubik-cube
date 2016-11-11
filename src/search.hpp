#ifndef __SEARCH_HPP__
#define __SEARCH_HPP__

#include "cube.h"
#include <queue>
#include <vector>
#include <cstdint>

namespace rubik_cube
{
	constexpr static int disallow_faces[6] = { -1, 0, -1, 2, -1, 4 };
	constexpr static int factorial_4[] = { 1, 4, 12, 24 };
	constexpr static int factorial_8[] = { 1, 8, 56, 336, 1680, 6720, 20160, 40320 };
	constexpr static int factorial_12[] = { 1, 12, 132, 1320, 11880, 95040, 665280, 3991680, 19958400, 79833600, 239500800, 479001600 };

	template<int N, int S>
	inline int encode_perm(const int8_t *p, const int *k) 
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

	template<bool IsGroup1, bool RecordState = false>
	inline void init_heuristic(
		int8_t *buf, 
		int(*encoder)(const cube_t&),
		std::vector<cube_t>* recorder = nullptr, 
		const std::vector<cube_t>& init_state = { cube_t() } )
	{
		std::queue<std::pair<cube_t, uint8_t>> que;

		for(const cube_t& c : init_state)
		{
			buf[(*encoder)(c)] = 0;
			que.push( { c, 0 | (6 << 4) } );
		}

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
					if(IsGroup1 && i >= 2)
					{
						if(j) break;
						c.rotate(face_t::face_type(i), 2);
					} else {
						c.rotate(face_t::face_type(i), 1);
					}

					int code = (*encoder)(c);
					if(buf[code] == -1)
					{
						buf[code] = step + 1;
						que.push( { c, (step + 1) | (i << 4) } );
						if(RecordState) recorder->push_back(c);
					}
				}
			}

			que.pop();
		}
	}
} // namespace rubik_rube

#endif // __SEARCH_HPP__
