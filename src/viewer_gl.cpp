#include "viewer.h"
#include <chrono>
#include <queue>
#include <algorithm>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

namespace rubik_cube
{

namespace __viewer_gl_impl
{

class rotate_manager_t
{
	typedef std::chrono::system_clock clock_type;
	std::chrono::time_point<clock_type> t_s;
	double t;
	bool active;
public:
	rotate_manager_t() : active(false) {}

	decltype(t_s) now()
	{
		return clock_type::now();
	}

	bool is_active()
	{
		return active;
	}

	double get()
	{
		std::chrono::duration<double> rate = now() - t_s;
		double r = rate.count() / t;
		if(r >= 1.0) active = false;
		return r;
	}

	void set(double duration)
	{
		t_s = now();
		t = duration;
		active = true;
	}
};

class viewer_gl : public viewer_t
{
public:
	viewer_gl();
	~viewer_gl();
public:
	void run();
	bool init(int&, char**&);
	void set_cube(const cube_t&);
	void set_rotate_duration(double);
	void add_rotate(face_t::face_type, int);
public:
	static void on_resize(GLFWwindow*, int, int);
private:
	void draw_cube();
	void draw_block(GLfloat x, GLfloat y, GLfloat z, GLfloat size, block_t, GLenum);
	void update_rotate();
	void set_color(int);
private:
	typedef std::pair<face_t::face_type, int> rotate_que_t;
	std::queue<rotate_que_t> rotate_que;

	int rotate_mask[3];
	GLfloat rotate_deg, rotate_vec;
	rotate_manager_t rotate_manager;
	double rotate_duration;

	cube_t cube;
	GLFWwindow* window;
};

viewer_gl::viewer_gl() 
{
	window = nullptr;
	rotate_duration = 1;
	rotate_deg = rotate_vec = 0;
	std::fill(rotate_mask, rotate_mask + 3, -1);
}

viewer_gl::~viewer_gl()
{
}

bool viewer_gl::init(int&, char**&)
{
	if(!glfwInit())
		return false;

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(600, 600, "Rubik's Cube", NULL, NULL);
	if(!window)
	{
		glfwTerminate();
		return false;
	}

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, on_resize);

	glfwMakeContextCurrent(window);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	return true;
}

void viewer_gl::run()
{
	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		update_rotate();
		draw_cube();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
}

void viewer_gl::set_cube(const cube_t& cube)
{
	this->cube = cube;
}

void viewer_gl::add_rotate(face_t::face_type type, int cnt)
{
	rotate_que.push( { type, cnt % 4 } );
}

void viewer_gl::update_rotate()
{
	if(rotate_que.empty())
		return;

	auto r = rotate_que.front();
	if(!rotate_manager.is_active())
	{
		rotate_manager.set(rotate_duration);
		std::fill(rotate_mask, rotate_mask + 3, -1);

		rotate_vec = r.second < 0 ? -1 : 1;

		switch(r.first)
		{
		case face_t::top:
			rotate_mask[0] = 2;
			break;
		case face_t::bottom:
			rotate_mask[0] = 0;
			break;
		case face_t::left:
			rotate_mask[2] = 0;
			rotate_vec = -rotate_vec;
			break;
		case face_t::right:
			rotate_mask[2] = 2;
			break;
		case face_t::front:
			rotate_mask[1] = 2;
			rotate_vec = -rotate_vec;
			break;
		case face_t::back:
			rotate_mask[1] = 0;
			break;
		}
	}

	double rate = rotate_manager.get();
	rotate_deg = std::abs(r.second) * 90.0 * rate;

	if(!rotate_manager.is_active())
	{
		std::fill(rotate_mask, rotate_mask + 3, -1);
		cube.rotate(r.first, r.second);
		rotate_que.pop();
	}
}

void viewer_gl::set_color(int type)
{
	static const GLfloat colors[7][3] = 
		{ { 1.0f, 0.0f, 0.0f }, // top
		  { 0.0f, 1.0f, 0.0f }, // front
		  { 0.4f, 0.4f, 1.0f }, // left
		  { 1.0f, 0.5f, 0.0f }, // back
		  { 1.0f, 1.0f, 1.0f }, // right
		  { 1.0f, 0.0f, 1.0f }, // bottom
		  { 0.0f, 0.0f, 0.0f }  // frame
		};

	const GLfloat *C = colors[type];
	glColor3f(C[0], C[1], C[2]);
}

void viewer_gl::set_rotate_duration(double sec)
{
	rotate_duration = sec;
}

void viewer_gl::draw_block(GLfloat x, GLfloat y, GLfloat z, GLfloat s, block_t color, GLenum type)
{
	set_color(color.back);
	glBegin(type);
		glVertex3f(x,     y,     z);
		glVertex3f(x,     y + s, z);
		glVertex3f(x + s, y + s, z);
		glVertex3f(x + s, y,     z);
	glEnd();

	set_color(color.front);
	glBegin(type);
		glVertex3f(x,     y,     z - s);
		glVertex3f(x,     y + s, z - s);
		glVertex3f(x + s, y + s, z - s);
		glVertex3f(x + s, y,     z - s);
	glEnd();

	set_color(color.top);
	glBegin(type);
		glVertex3f(x,     y + s, z);
		glVertex3f(x + s, y + s, z);
		glVertex3f(x + s, y + s, z - s);
		glVertex3f(x,     y + s, z - s);
	glEnd();

	set_color(color.bottom);
	glBegin(type);
		glVertex3f(x,     y, z);
		glVertex3f(x + s, y, z);
		glVertex3f(x + s, y, z - s);
		glVertex3f(x,     y, z - s);
	glEnd();

	set_color(color.left);
	glBegin(type);
		glVertex3f(x, y,     z);
		glVertex3f(x, y + s, z);
		glVertex3f(x, y + s, z - s);
		glVertex3f(x, y,     z - s);
	glEnd();

	set_color(color.right);
	glBegin(type);
		glVertex3f(x + s, y,     z);
		glVertex3f(x + s, y + s, z);
		glVertex3f(x + s, y + s, z - s);
		glVertex3f(x + s, y,     z - s);
	glEnd();
}

void viewer_gl::draw_cube()
{
	class rotate_guard
	{
		bool is_rotated;
	public:
		rotate_guard(int mask, int real, GLfloat deg, GLfloat X, GLfloat Y, GLfloat Z)
		{
			if(mask == real)
			{
				is_rotated = true;
				glPushMatrix();
				glRotatef(deg, X, Y, Z);
			} else is_rotated = false;
		}

		~rotate_guard()
		{
			if(is_rotated)
				glPopMatrix();
		}
	};

	GLfloat size = 0.25f;

	glPushMatrix();

	glRotatef(45.0f, -1.0f, 1.0f, 0.0f);
	glLineWidth(1.5f);

	GLfloat base = -size * 1.5f, x, y, z;
	x = y = base, z = -base;

	for(int i = 0; i != 3; ++i, y += size, x = base, z = -base)
	{
		rotate_guard _guard(rotate_mask[0], i, rotate_deg, 0, rotate_vec, 0);
		for(int j = 0; j != 3; ++j, z -= size, x = base)
		{
			rotate_guard _guard(rotate_mask[1], j, rotate_deg, 0, 0, rotate_vec);
			for(int k = 0; k != 3; ++k, x += size)
			{
				rotate_guard _guard(rotate_mask[2], k, rotate_deg, rotate_vec, 0, 0);
				draw_block(x, y, z, size, cube.getBlock(i, j, k), GL_QUADS);
				draw_block(x, y, z, size, { 6, 6, 6, 6, 6, 6 }, GL_LINE_LOOP);
			}
		}
	}

	glPopMatrix();
}

void viewer_gl::on_resize(GLFWwindow* window, int w, int h)
{
	glfwMakeContextCurrent(window);

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

} // namespace __viewer_gl_impl

viewer_t* create_opengl_viewer()
{
	return new __viewer_gl_impl::viewer_gl;
}

void destroy_opengl_viewer(viewer_t* viewer)
{
	delete viewer;
}

} // namespace rubik_cube
