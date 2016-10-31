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
	virtual bool init(int&, char**&) = 0;
	virtual void display(const cube_t&) = 0;
};

viewer_t *create_opengl_viewer();
void destroy_opengl_viewer(viewer_t*);
}

#endif // __VIEWER_H__
