#ifndef QUATERNION_H
#define QUATERNION_H

struct Quaternion {
	double x, y, z, w;

	Quaternion(double x = 0, double y = 0, double z = 0, double w = 0);
	~Quaternion();

	double length();
	
	Quaternion conjugate();

	Quaternion multiply(Quaternion q);

	void normalize();
	void setVector(double vx, double vy, double vz);
	void setScalar(double s);

};

#endif

