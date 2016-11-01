#ifndef __VIRTUAL_ROTATE_H__
#define __VIRTUAL_ROTATE_H__
#include <GL/gl.h>
#include <cmath>
#include <algorithm>

namespace rubik_cube
{
	class virtual_ball_t
	{
		struct vector3d
		{
			double x, y, z;

			double operator * (const vector3d& r) const
			{
				return x * r.x + y * r.y + z * r.z;
			}

			vector3d cross(const vector3d& r) const
			{
				return { y * r.z - z * r.y,
					     x * r.z - z * r.x,
						 y * r.x - x * r.y };
			}

			double norm() const
			{
				return sqrt((*this) * (*this));
			}
		};

		bool is_started;
		double R, deg;
		vector3d start, end, axis;
		GLfloat M[16];

		vector3d screen2ball(double x, double y)
		{
			double len = sqrt(x * x + y * y);
			if(len > R) return { R * x / len, R * y / len, 0 };
			return { x, y, sqrt(R * R - x * x - y * y) };
		}

		void clear()
		{
			deg = 0;
			axis.x = 1;
			axis.y = axis.z = 0;
		}

	public:
		virtual_ball_t()
		{
			R = 0.5;
			is_started = false;
			clear();
		}

		operator bool() const
		{
			return is_started;
		}

		void set_rotate(double deg, vector3d v)
		{
			clear();
			glPushMatrix();
			glRotated(deg, v.x, v.y, v.z);
			glGetFloatv(GL_MODELVIEW_MATRIX, M);
			glPopMatrix();
		}

		void set_start(double x, double y)
		{
			is_started = true;
			start = screen2ball(x, y);
		}

		void set_middle(double x, double y)
		{
			static const double coeff = 180.0 / acos(-1.0);
			end = screen2ball(x, y);
			double alpha = acos((start * end) / start.norm() / end.norm());
			deg = alpha * coeff;

			vector3d v = start.cross(end);
			axis.x = v.x * M[0] + v.y * M[1] + v.z * M[2];
			axis.y = v.x * M[4] + v.y * M[5] + v.z * M[6];
			axis.z = v.x * M[8] + v.y * M[9] + v.z * M[10];
		}

		void set_end(double x, double y)
		{
			is_started = false;
			set_middle(x, y);

			glPushMatrix();
			glLoadMatrixf(M);
			glRotated(deg, axis.x, axis.y, axis.z);
			glGetFloatv(GL_MODELVIEW_MATRIX, M);
			glPopMatrix();
			clear();
		}

		void rotate()
		{
			glMultMatrixf(M);
			glRotated(deg, axis.x, axis.y, axis.z);
		}
	};
}

#endif // __VIRTUAL_ROTATE_H__
