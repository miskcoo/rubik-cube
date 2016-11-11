#include "cube.h"
#include <cstring>

namespace rubik_cube
{

cube_t::cube_t()
{
	std::memset(co, 0, sizeof(co));
	std::memset(eo, 0, sizeof(eo));
	for(int i = 0; i != 8; ++i)
		cp[i] = i;
	for(int i = 0; i != 12; ++i)
		ep[i] = i;
}

void cube_t::rotate(face_t::face_type type, int count)
{
	static const int corner_rotate_map[2][6][4] = 
	{
		{
			// clockwise
			{ 4, 5, 6, 7 }, // top
			{ 3, 2, 1, 0 }, // bottom
			{ 7, 6, 2, 3 }, // front
			{ 5, 4, 0, 1 }, // back
			{ 4, 7, 3, 0 }, // left
			{ 6, 5, 1, 2 }  // right
		}, {
			// counterclockwise
			{ 7, 6, 5, 4 }, // top
			{ 0, 1, 2, 3 }, // bottom
			{ 7, 3, 2, 6 }, // front
			{ 5, 1, 0, 4 }, // back
			{ 4, 0, 3, 7 }, // left
			{ 6, 2, 1, 5 }  // right
		}
	};

	static const int edge_rotate_map[2][6][4] = 
	{
		{
			// clockwise
			{ 4, 5, 6, 7 },   // top
			{ 11, 10, 9, 8 }, // bottom
			{ 6, 2, 10, 3 },  // front
			{ 4, 0, 8, 1 },   // back
			{ 7, 3, 11, 0 },  // left
			{ 5, 1, 9, 2 }    // right
		}, {
			// counterclockwise
			{ 7, 6, 5, 4 },   // top
			{ 8, 9, 10, 11 }, // bottom
			{ 3, 10, 2, 6 },  // front
			{ 1, 8, 0, 4 },   // back
			{ 0, 11, 3, 7 },  // left
			{ 2, 9, 1, 5 }    // right
		}
	};

	static const auto swap = [](int8_t *A, const int *C) {
		int tmp = A[C[3]];
		A[C[3]] = A[C[2]];
		A[C[2]] = A[C[1]];
		A[C[1]] = A[C[0]];
		A[C[0]] = tmp;
	};

	static const auto swap2 = [](int8_t &a, int8_t &b) {
		int8_t t = a;
		a = b;
		b = t;
	};

	count = (count % 4 + 4) & 3;

	if(count == 2)
	{
		const int *C = corner_rotate_map[0][int(type)];

		swap2(cp[C[0]], cp[C[2]]);
		swap2(cp[C[1]], cp[C[3]]);
		swap2(co[C[0]], co[C[2]]);
		swap2(co[C[1]], co[C[3]]);

		const int *E = edge_rotate_map[0][int(type)];
		swap2(ep[E[0]], ep[E[2]]);
		swap2(ep[E[1]], ep[E[3]]);
		swap2(eo[E[0]], eo[E[2]]);
		swap2(eo[E[1]], eo[E[3]]);
	} else {
		// rotate the corners
		const int *C = corner_rotate_map[count >> 1][int(type)];
		swap(cp, C); swap(co, C);

		// rotation of top and bottom face does not change the orientation
		if(int(type) >= 2) 
		{
			if(++co[C[0]] == 3) co[C[0]] = 0;
			if(++co[C[2]] == 3) co[C[2]] = 0;
			if(--co[C[1]] == -1) co[C[1]] = 2;
			if(--co[C[3]] == -1) co[C[3]] = 2;
		}

		// rotate the edges
		const int *E = edge_rotate_map[count >> 1][int(type)];
		if(int(type) >= 4)
		{
			eo[E[0]] ^= 1;
			eo[E[1]] ^= 1;
			eo[E[2]] ^= 1;
			eo[E[3]] ^= 1;
		}

		swap(ep, E); swap(eo, E);
	}
}

block_t cube_t::getBlock(int level, int x, int y) const
{
	static const int corner_orient_map[][3] = 
	{
		{ 1, 3, 4 }, // (0, 0, 0)
		{ 1, 5, 3 }, // (0, 0, 2)
		{ 1, 2, 5 }, // (0, 2, 2)
		{ 1, 4, 2 }, // (0, 2, 0)
		{ 0, 4, 3 }, // (2, 0, 0)
		{ 0, 3, 5 }, // (2, 0, 2)
		{ 0, 5, 2 }, // (2, 2, 2)
		{ 0, 2, 4 }  // (2, 2, 0)
	};

	static const int edge_orient_map[][2] = 
	{
		{ 4, 3 }, { 5, 3 }, { 5, 2 }, { 4, 2 },
		{ 0, 3 }, { 0, 5 }, { 0, 2 }, { 0, 4 },
		{ 1, 3 }, { 1, 5 }, { 1, 2 }, { 1, 4 },
	}; 

	static const int edge_id_map[27] = 
	{
		-1, 8, -1, 11, -1, 9, -1, 10, -1,
		0, -1, 1, -1, -1, -1, 3, -1, 2,
		-1, 4, -1, 7, -1, 5, -1, 6, -1
	};

	static const int center_id_map[27] = 
	{
		-1, -1, -1, -1, 1, -1, -1, -1, -1,
		-1, 3, -1, 4, -1, 5, -1, 2, -1,
		-1, -1, -1, -1, 0, -1, -1, -1, -1,
	};

	int nid = level * 9 + x * 3 + y;
	int8_t F[6] = { -1, -1, -1, -1, -1, -1 };

	if(level != 1 && x != 1 && y != 1)
	{
		int id = (level << 1) | x | ((x ^ y) >> 1);
		const int *O = corner_orient_map[id];
		const int *C = corner_orient_map[cp[id]];
		F[O[0]] = C[co[id]];
		F[O[1]] = C[(1 + co[id]) % 3];
		F[O[2]] = C[(2 + co[id]) % 3];
	} else if(edge_id_map[nid] != -1) {
		int id = edge_id_map[nid];
		const int *O = edge_orient_map[id];
		const int *C = edge_orient_map[ep[id]];
		F[O[0]] = C[eo[id]];
		F[O[1]] = C[eo[id] ^ 1];
	} else if(level != 1 || x != 1 || y != 1) {
		F[center_id_map[nid]] = center_id_map[nid];
	}

	return { F[0], F[1], F[2], F[3], F[4], F[5] };
}

block_info_t cube_t::getCornerBlock() const
{
	return { cp, co };
}

block_info_t cube_t::getEdgeBlock() const
{
	return { ep, eo };
}

}
