#include "cube4.h"
#include <cstring>

namespace rubik_cube
{

cube4_t::cube4_t()
{
	std::memset(co, 0, sizeof(co));
	std::memset(eo, 0, sizeof(eo));
	for(int i = 0; i != 8; ++i)
		cp[i] = i;
	for(int i = 0; i != 24; ++i)
	{
		ep[i] = i >> 1;
		fp[i] = i >> 2;
	}
}

void cube4_t::rotate(face_t::face_type type, int depth, int count)
{
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

	if(depth == 1)
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
				{ 8, 10, 12, 14 },  // top
				{ 22, 20, 18, 16 }, // bottom
				{ 12, 5, 21, 6 },   // front
				{ 8, 1, 17, 2 },    // back
				{ 14, 7, 23, 0 },   // left
				{ 10, 3, 19, 4 }    // right
			}, {
				// counterclockwise
				{ 14, 12, 10, 8 },  // top
				{ 16, 18, 20, 22 }, // bottom
				{ 6, 21, 5, 12 },   // front
				{ 2, 17, 1, 8 },    // back
				{ 0, 23, 7, 14 },   // left
				{ 4, 19, 3, 10 }    // right
			}
		};

		int f0 = int(type) * 4;
		if(count == 2)
		{
			const int *C = corner_rotate_map[0][int(type)];

			swap2(cp[C[0]], cp[C[2]]);
			swap2(cp[C[1]], cp[C[3]]);
			swap2(co[C[0]], co[C[2]]);
			swap2(co[C[1]], co[C[3]]);

			const int *E = edge_rotate_map[0][int(type)];
			swap2(ep[E[0]], ep[E[2]]); swap2(ep[E[0] ^ 1], ep[E[2] ^ 1]);
			swap2(ep[E[1]], ep[E[3]]); swap2(ep[E[1] ^ 1], ep[E[3] ^ 1]);
			swap2(eo[E[0]], eo[E[2]]); swap2(eo[E[0] ^ 1], eo[E[2] ^ 1]);
			swap2(eo[E[1]], eo[E[3]]); swap2(eo[E[1] ^ 1], eo[E[3] ^ 1]);

			swap2(fp[f0 + 0], fp[f0 + 2]);
			swap2(fp[f0 + 1], fp[f0 + 3]);
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
			const int E1[4] = { E[0] ^ 1, E[1] ^ 1, E[2] ^ 1, E[3] ^ 1 };

			if(int(type) >= 4)
			{
				eo[E[0]] ^= 1; eo[E[0] ^ 1] ^= 1;
				eo[E[1]] ^= 1; eo[E[1] ^ 1] ^= 1;
				eo[E[2]] ^= 1; eo[E[2] ^ 1] ^= 1;
				eo[E[3]] ^= 1; eo[E[3] ^ 1] ^= 1;
			}

			swap(ep, E); swap(eo, E);
			swap(ep, E1); swap(eo, E1);

			// rotate the faces
			const int F[][4] = { { f0, f0 + 1, f0 + 2, f0 + 3 }, { f0 + 3, f0 + 2, f0 + 1, f0 } };
			swap(fp, F[count >> 1]);
		}
	} else if(depth == 2) {
		static const int edge_rotate_map[2][6][4] = 
		{
			{
				// clockwise
				{ 0, 2, 4, 6 },  // top
				{ 7, 5, 3, 1 }, // bottom
				{ 11, 19, 22, 14 },   // front
				{ 15, 23, 18, 10 },    // back
				{ 8, 13, 21, 16 },   // left
				{ 12, 9, 17, 20 }    // right
			}, {
				// counterclockwise
				{ 6, 4, 2, 0 },  // top
				{ 1, 3, 5, 7 }, // bottom
				{ 14, 22, 19, 11 },   // front
				{ 10, 18, 23, 15 },    // back
				{ 16, 21, 13, 8 },   // left
				{ 20, 17, 9, 12 }    // right
			}
		};

		static const int face_rotate_map[2][6][2][4] = 
		{
			{
				// clockwise
				{ { 12, 20, 8, 16 }, { 13, 21, 9, 17 } },   // top
				{ { 10, 22, 14, 18 }, { 11, 23, 15, 19 } }, // bottom
				{ { 2, 23, 6, 17 }, { 3, 20, 7, 18 } }, // front
				{ { 0, 19, 4, 21 }, { 1, 16, 5, 22 } }, // back
				{ { 13, 3, 11, 5 }, { 14, 0, 8, 6 } },  // left
				{ { 2, 12, 4, 10 }, { 1, 15, 7, 9 } }   // right
			}, {
				// counterclockwise
				{ { 16, 8, 20, 12 }, { 17, 9, 21, 13 } },   // top
				{ { 18, 14, 22, 10 }, { 19, 15, 23, 11 } }, // bottom
				{ { 17, 6, 23, 2 }, { 18, 7, 20, 3 } }, // front
				{ { 21, 4, 19, 0 }, { 22, 5, 16, 1 } }, // back
				{ { 5, 11, 3, 13 }, { 6, 8, 0, 14 } },  // left
				{ { 10, 4, 12, 2 }, { 9, 7, 15, 1 } }   // right
			}
		};

		if(count == 2)
		{
			const int *E = edge_rotate_map[0][int(type)];
			swap2(ep[E[0]], ep[E[2]]);
			swap2(ep[E[1]], ep[E[3]]);
			swap2(eo[E[0]], eo[E[2]]);
			swap2(eo[E[1]], eo[E[3]]);

			const int (*F)[4] = face_rotate_map[count >> 1][int(type)];
			swap2(fp[F[0][0]], fp[F[0][2]]);
			swap2(fp[F[0][1]], fp[F[0][3]]);
			swap2(fp[F[1][0]], fp[F[1][2]]);
			swap2(fp[F[1][1]], fp[F[1][3]]);
		} else {
			// rotate the edges
			const int *E = edge_rotate_map[count >> 1][int(type)];
			eo[E[0]] ^= 1;
			eo[E[1]] ^= 1;
			eo[E[2]] ^= 1;
			eo[E[3]] ^= 1;
			swap(ep, E); swap(eo, E);

			// rotate the faces
			const int (*F)[4] = face_rotate_map[count >> 1][int(type)];
			swap(fp, F[0]); swap(fp, F[1]);
		}
	}
}

block_t cube4_t::getBlock(int level, int x, int y) const
{
	static const int corner_orient_map[][3] = 
	{
		{ 1, 3, 4 }, // (0, 0, 0)
		{ 1, 5, 3 }, // (0, 0, 3)
		{ 1, 2, 5 }, // (0, 3, 3)
		{ 1, 4, 2 }, // (0, 3, 0)
		{ 0, 4, 3 }, // (3, 0, 0)
		{ 0, 3, 5 }, // (3, 0, 3)
		{ 0, 5, 2 }, // (3, 3, 3)
		{ 0, 2, 4 }  // (3, 3, 0)
	};

	static const int edge_orient_map[][2] = 
	{
		{ 4, 3 }, { 5, 3 }, { 5, 2 }, { 4, 2 },
		{ 0, 3 }, { 0, 5 }, { 0, 2 }, { 0, 4 },
		{ 1, 3 }, { 1, 5 }, { 1, 2 }, { 1, 4 },
	}; 

	static const int edge_id_map[64] = 
	{
		-1, 16, 17, -1, 23, -1, -1, 18, 22, -1, -1, 19, -1, 21, 20, -1,
		1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1, 7, -1, -1, 5,
		0, -1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, 6, -1, -1, 4,
		-1, 8, 9, -1, 15, -1, -1, 10, 14, -1, -1, 11, -1, 13, 12, -1
	};

	static const int center_id_map[64] = 
	{
		-1, -1, -1, -1, -1, 5, 4, -1, -1, 6, 7, -1, -1, -1, -1, -1,
		-1, 14, 15, -1, 19, -1, -1, 22, 18, -1, -1, 23, -1, 11, 10, -1,
		-1, 13, 12, -1, 16, -1, -1, 21, 17, -1, -1, 20, -1, 8, 9, -1,
		-1, -1, -1, -1, -1, 0, 1, -1, -1, 3, 2, -1, -1, -1, -1, -1,
	};

	int nid = level * 16 + x * 4 + y;
	int8_t F[6] = { -1, -1, -1, -1, -1, -1 };

	auto check0 = [] ( int x ) { return x != 1 && x != 2; };

	if(check0(x) && check0(y) && check0(level))
	{
		if(x == 3) x = 2;
		if(y == 3) y = 2;
		if(level == 3) level = 2;

		int id = (level << 1) | x | ((x ^ y) >> 1);
		const int *O = corner_orient_map[id];
		const int *C = corner_orient_map[cp[id]];
		F[O[0]] = C[co[id]];
		F[O[1]] = C[(1 + co[id]) % 3];
		F[O[2]] = C[(2 + co[id]) % 3];
	} else if(edge_id_map[nid] != -1) {
		int id = edge_id_map[nid];
		const int *O = edge_orient_map[id >> 1];
		const int *C = edge_orient_map[ep[id]];
		F[O[0]] = C[eo[id]];
		F[O[1]] = C[eo[id] ^ 1];
	} else if(center_id_map[nid] != -1) {
		F[center_id_map[nid] >> 2] = fp[center_id_map[nid]];
	}

	return { F[0], F[1], F[2], F[3], F[4], F[5] };
}

block_info_t cube4_t::getCornerBlock() const
{
	return { cp, co };
}

block_info_t cube4_t::getEdgeBlock() const
{
	return { ep, eo };
}

const int8_t* cube4_t::getFaceBlock() const
{
	return fp;
}

}
