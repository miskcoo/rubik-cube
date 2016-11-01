#ifndef __VIEWER_H__
#define __VIEWER_H__

#include "cube.h"

namespace rubik_cube
{
class viewer_t
{
public:
	virtual ~viewer_t() = default;
public:
	virtual void run() = 0;
	virtual bool init(int&, char**&) = 0;
	virtual void set_cube(const cube_t&) = 0;
	virtual void set_rotate_duration(double second) = 0;
	virtual void add_rotate(face_t::face_type, int) = 0;
};

viewer_t *create_opengl_viewer();
void destroy_opengl_viewer(viewer_t*);
}

#endif // __VIEWER_H__
