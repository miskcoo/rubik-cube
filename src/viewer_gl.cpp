#include "viewer.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

namespace rubik_cube
{

namespace __viewer_gl_impl
{

class viewer_gl : public viewer_t
{
public:
	~viewer_gl() {}
public:
	bool init(int&, char**&);
	void display(const cube_t&);
public:
	static void on_resize(GLFWwindow*, int, int);
private:
	void draw_cube();
	void draw_block(GLfloat x, GLfloat y, GLfloat z, GLfloat size, block_t);
	void set_color(int);
private:
	cube_t cube;
	GLFWwindow* window;
};

bool viewer_gl::init(int&, char**&)
{
	if(!glfwInit())
		return false;

	window = glfwCreateWindow(600, 600, "Rubik's Cube", NULL, NULL);
	if(!window)
	{
		glfwTerminate();
		return false;
	}

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, on_resize);

	glfwMakeContextCurrent(window);
	glEnable(GL_DEPTH_TEST);

	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_cube();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return true;
}

void viewer_gl::display(const cube_t& cube)
{
	this->cube = cube;
}

void viewer_gl::set_color(int type)
{
	static const GLfloat colors[6][3] = 
		{ { 1.0f, 0.0f, 0.0f }, // top
		  { 0.0f, 1.0f, 0.0f }, // front
		  { 0.4f, 0.4f, 1.0f }, // left
		  { 1.0f, 0.5f, 0.0f }, // back
		  { 1.0f, 1.0f, 1.0f }, // right
		  { 1.0f, 0.0f, 1.0f }  // bottom
		};

	const GLfloat *C = colors[type];
	glColor3f(C[0], C[1], C[2]);
}

void viewer_gl::draw_block(GLfloat x, GLfloat y, GLfloat z, GLfloat s, block_t color)
{
	set_color(color.back);
	glBegin(GL_QUADS);
		glVertex3f(x,     y,     z);
		glVertex3f(x,     y + s, z);
		glVertex3f(x + s, y + s, z);
		glVertex3f(x + s, y,     z);
	glEnd();

	set_color(color.front);
	glBegin(GL_QUADS);
		glVertex3f(x,     y,     z - s);
		glVertex3f(x,     y + s, z - s);
		glVertex3f(x + s, y + s, z - s);
		glVertex3f(x + s, y,     z - s);
	glEnd();

	set_color(color.top);
	glBegin(GL_QUADS);
		glVertex3f(x,     y + s, z);
		glVertex3f(x + s, y + s, z);
		glVertex3f(x + s, y + s, z - s);
		glVertex3f(x,     y + s, z - s);
	glEnd();

	set_color(color.bottom);
	glBegin(GL_QUADS);
		glVertex3f(x,     y, z);
		glVertex3f(x + s, y, z);
		glVertex3f(x + s, y, z - s);
		glVertex3f(x,     y, z - s);
	glEnd();

	set_color(color.left);
	glBegin(GL_QUADS);
		glVertex3f(x, y,     z);
		glVertex3f(x, y + s, z);
		glVertex3f(x, y + s, z - s);
		glVertex3f(x, y,     z - s);
	glEnd();

	set_color(color.right);
	glBegin(GL_QUADS);
		glVertex3f(x + s, y,     z);
		glVertex3f(x + s, y + s, z);
		glVertex3f(x + s, y + s, z - s);
		glVertex3f(x + s, y,     z - s);
	glEnd();
}

void viewer_gl::draw_cube()
{
	GLfloat size = 0.25f, bias = size * 0.02f;

	glPushMatrix();

	glRotatef(45.0f, -1.0f, 1.0f, 0.0f);

	GLfloat base = -(size + bias) * 1.5f, x, y, z;
	x = y = base, z = -base;

	for(int i = 0; i != 3; ++i, y += size + bias, x = base, z = -base)
		for(int j = 0; j != 3; ++j, z -= size + bias, x = base)
			for(int k = 0; k != 3; ++k, x += size + bias)
				draw_block(x, y, z, size, cube.getBlock(i, j, k));


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
