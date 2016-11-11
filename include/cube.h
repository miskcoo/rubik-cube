/** 
    Structure and basic manipulations of Rubik's Cube.
 **/
#ifndef __CUBE_H__
#define __CUBE_H__

#include <utility>
#include <cstdint>

namespace rubik_cube
{

struct face_t
{
	enum face_type
	{
		top    = 0,
		bottom = 1,
		front  = 2,
		back   = 3,
		left   = 4,
		right  = 5,
	};

	int8_t C[9];
};

struct block_t
{
	int8_t top, bottom, front, back, left, right;
};

typedef std::pair<const int8_t*, const int8_t*> block_info_t;

/*
 * observing from the top face, the index of corners will be like this
 *     *-*-*-*        *-*-*-*
 *     |0| |1|        |4| |5|
 *     *-*-*-*        *-*-*-*
 *     | | | |        | | | |
 *     *-*-*-*        *-*-*-*
 *     |3| |2|        |7| |6|
 *     *-*-*-*        *-*-*-*
 * the bottom face, the top face
 *
 * observing from the top face, the index of edges will be like this
 *     *-*-*-*          *-*-*-*         *-*-*-*
 *     | |8| |          |0| |1|         | |4| |
 *     *-*-*-*          *-*-*-*         *-*-*-*
 *     |B| |9|          | | | |         |7| |5|
 *     *-*-*-*          *-*-*-*         *-*-*-*
 *     | |A| |          |3| |2|         | |6| |
 *     *-*-*-*          *-*-*-*         *-*-*-*
 * the bottom face, the middle level, the top face
 *
 * the priority of the key faces: UD > LR > FB
 *
 */
class cube_t
{
public:
	cube_t();
public:
	/* look from the top face
	 * *-----------------*
	 * |(0,0) (0,1) (0,2)|
	 * |(1,0) (1,1) (1,2)|
	 * |(2,0) (2,1) (2,2)|
	 * *-----------------*
	 *
	 * bottom level = 0
	 * middle level = 1
	 * top level = 2
	 */
	block_t getBlock(int level, int x, int y) const;

	block_info_t getCornerBlock() const;
	block_info_t getEdgeBlock() const;

	/* rotate a face 90 * count degree clockwise */
	void rotate(face_t::face_type, int count = 1);
private:
	int8_t cp[8], co[8];   // corners' position and orientation
	int8_t ep[12], eo[12]; // edges' position and orientation
};
}

#endif // __CUBE_H__
