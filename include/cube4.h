/** 
    Structure and basic manipulations of Rubik's Revenge.
 **/
#ifndef __CUBE4_H__
#define __CUBE4_H__

#include "cube.h"

namespace rubik_cube
{

/*
 * observing from the top face, the index of corners will be like this
 *     *-*-*-*-*     *-*-*-*-*
 *     |0| | |1|     |4| | |5|
 *     *-*-*-*-*     *-*-*-*-*
 *     | | | | |     | | | | |
 *     *-*-*-*-*     *-*-*-*-*
 *     | | | | |     | | | | |
 *     *-*-*-*-*     *-*-*-*-*
 *     |3| | |2|     |7| | |6|
 *     *-*-*-*-*     *-*-*-*-*
 * the bottom face, the top face
 *
 * observing from the top face, the index of edges will be like this
 *     *--*--*--*--*    *-*-*-*-*    *-*-*-*-*    *--*--*--*--*
 *     |  |16|17|  |    |1| | |3|    |0| | |2|    |  |8 |9 |  |
 *     *--*--*--*--*    *-*-*-*-*    *-*-*-*-*    *--*--*--*--*
 *     |23|  |  |18|    | | | | |    | | | | |    |15|  |  |10|
 *     *--*--*--*--*    *-*-*-*-*    *-*-*-*-*    *--*--*--*--*
 *     |22|  |  |19|    | | | | |    | | | | |    |14|  |  |11|
 *     *--*--*--*--*    *-*-*-*-*    *-*-*-*-*    *--*--*--*--*
 *     |  |21|20|  |    |7| | |5|    |6| | |4|    |  |13|12|  |
 *     *--*--*--*--*    *-*-*-*-*    *-*-*-*-*    *--*--*--*--*
 *    the bottom face       the middle faces        the top face
 *
 * observing from the top face, the index of faces will be like this
 *     *-*-*-*-*    *--*--*--*--*    *--*--*--*--*    *-*-*-*-*
 *     | | | | |    |  |14|15|  |    |  |13|12|  |    | | | | |
 *     *-*-*-*-*    *--*--*--*--*    *--*--*--*--*    *-*-*-*-*
 *     | |5|4| |    |19|  |  |22|    |16|  |  |21|    | |0|1| |
 *     *-*-*-*-*    *--*--*--*--*    *--*--*--*--*    *-*-*-*-*
 *     | |6|7| |    |18|  |  |23|    |17|  |  |20|    | |3|2| |
 *     *-*-*-*-*    *--*--*--*--*    *--*--*--*--*    *-*-*-*-*
 *     | | | | |    |  |11|10|  |    |  |8 |9 |  |    | | | | |
 *     *-*-*-*-*    *--*--*--*--*    *--*--*--*--*    *-*-*-*-*
 *  the bottom face       the middle faces          the top face
 *
 * the priority of the key faces: UD > LR > FB
 *
 */
class cube4_t
{
public:
	cube4_t();
public:
	/* look from the top face
	 * *-----------------------*
	 * |(0,0) (0,1) (0,2) (0,3)|
	 * |(1,0) (1,1) (1,2) (1,3)|
	 * |(2,0) (2,1) (2,2) (2,3)|
	 * |(3,0) (3,1) (3,2) (3,3)|
	 * *-----------------------*
	 *
	 * bottom level = 0
	 */
	block_t getBlock(int level, int x, int y) const;

	block_info_t getCornerBlock() const;
	block_info_t getEdgeBlock() const;
	const int8_t* getFaceBlock() const;

	/* rotate a face 90 * count degree clockwise */
	void rotate(face_t::face_type, int depth = 1, int count = 1);
private:
	int8_t cp[8], co[8];   // corners' position and orientation
	int8_t ep[24], eo[24]; // edges' position and orientation
	int8_t fp[24];         // faces' position 
};
}

#endif // __CUBE4_H__
