#include "quaternion.h"
#include <math.h>



Quaternion::Quaternion(double x, double y, double z, double w)
{
	setVector(x, y, z);
	setScalar(w);
}


Quaternion::~Quaternion()
{
}

void Quaternion::setVector(double vx, double vy, double vz) {
	x = vx;
	y = vy;
	z = vz;
}

void Quaternion::setScalar(double s) {
	w = s;
}

double Quaternion::length() {
	return sqrt(x * x + y * y + z * z);
}

void Quaternion::normalize() {
	double l = length();
	x /= l;
	y /= l;
	z /= l;
	w /= l;
}

Quaternion Quaternion::conjugate() {
	return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::multiply(Quaternion q) {
	double nx, ny, nz, nw;
	nx = w * q.x + x * q.w + y * q.z + z * q.y;
	ny = w * q.y + x * q.z + y * q.w + z * q.x;
	nz = w * q.z + x * q.y + y * q.x + z * q.w;
	nw = w * q.w + x * q.w + y * q.y + z * q.z;

	return Quaternion(nx, ny, nz, nw);
}

