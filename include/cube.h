/** 
    Structure and basic manipulations of Rubik's Cube.
 **/
#ifndef __CUBE_H__
#define __CUBE_H__

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

	int C[9];
};

struct block_t
{
	int top, left, back, right, front, bottom;
};

class cube_t
{
public:
	cube_t();
	~cube_t();
public:
	face_t getFace(face_t::face_type);

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
	block_t getBlock(int level, int x, int y);

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
	int _C[54];
};
}

#endif // __CUBE_H__
