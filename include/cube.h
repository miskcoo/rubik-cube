/** 
    Structure and basic manipulations of Rubik's Cube.
 **/
#ifndef __CUBE_H__
#define __CUBE_H__

#include <cstdint>

namespace rubik_cube
{

struct face_t
{
	enum face_type
	{
		top    = 0,
		front  = 1,
		left   = 2,
		back   = 3,
		right  = 4,
		bottom = 5
	};

	int8_t C[9];
};

struct block_t
{
	int8_t top, left, back, right, front, bottom, id;
};

struct corner_block_t
{
	int permutation[8];
	int8_t top_bottom_color[8];
};

struct edge_block_t
{
	int permutation[12];
	int8_t color[12];
};

class cube_t
{
public:
	cube_t();
	~cube_t();
public:
	face_t getFace(face_t::face_type) const;

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

	corner_block_t getCornerBlock() const;
	edge_block_t getEdgeBlock() const;

	/* rotate a face 90 * count degree clockwise */
	void rotate(face_t::face_type, int count = 1);
private:
	/*          *--------*
	 *          |32 33 34|
	 *          |31 27 35|
	 *          |30 29 28|
	 * *--------*--------*--------*--------*
	 * |25 26 19| 1  2  3|39 40 41|46 47 48|
	 * |24 18 20| 8  0  4|38 36 42|53 45 49|
	 * |23 22 21| 7  6  5|37 44 43|52 51 50|
	 * *--------*--------*--------*--------*
	 *          |10 11 12|
	 *          |17  9 13|
	 *          |16 15 14|
	 *          *--------*
	 *
	 * The top face is numbered from 0 to 8;
	 * The bottom face is numbered from 45 to 53;
	 * The front face is numbered from 9 to 17;
	 * The color ranges from 0 to 5.
	 */
	int8_t _C[54];
	/* *-------*  *--------*   *--------*
	 * |0  8  1|  |12 23 13|   | 4 16  5|
	 * |11 20 9|  |26 21 24|   |19 22 17|
	 * |3  10 2|  |15 25 14|   | 7 18  6|
	 * *-------*  *--------*   *--------*
	 *  bottom      middle        top
	 */
	int8_t _B[20];
};
}

#endif // __CUBE_H__
