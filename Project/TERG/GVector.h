#pragma once
#include <vector>
class GVector
{
public:
	GVector();
	~GVector();
};

using namespace std;

static int vectorContainsIndex = -1;

template<typename T, typename A>
static bool vectorContains(vector<T, A> const& v, T const& val) {
	for (int i = 0; i < v.size(); i++)
		if (v[i] == val) {
			vectorContainsIndex = i;
			return true;
		}
	return false;
}

template<typename T, typename A>
static bool vectorContainsPointedObject(vector<T, A> const& v, T const& val) {
	for (int i = 0; i < v.size(); i++)
		if (*v[i] == *val) {
			vectorContainsIndex = i;
			return true;
		}
	return false;
}

struct vector2f {
	float x, y;
	vector<vector2f> transformations;

	vector2f(float sx = 0, float sy = 0) {
		x = sx;
		y = sy;
	}

	void transform(vector2f vector)
	{
		x += vector.x;
		y += vector.y;
		transformations.push_back(vector);
	}

	bool operator == (const vector2f& v) const {
		return x == v.x && y == v.y;
	}

	vector2f operator - (const vector2f& v) const {
		return vector2f(x - v.x, y - v.y);
	}
};

struct vector3f {
	float x, y, z;
	vector<vector3f> transformations;

	vector3f(float sx = 0, float sy = 0, float sz = 0) {
		x = sx;
		y = sy;
		z = sz;
	}

	void transform(vector3f vector)
	{
		x += vector.x;
		y += vector.y;
		z += vector.z;
		transformations.push_back(vector);
	}

	bool operator == (const vector3f& v) const {
		return x == v.x && y == v.y && z == v.z;
	}

	bool operator != (const vector3f& v) const {
		return x != v.x || y != v.y || z != v.z;
	}

	vector3f operator - (const vector3f& v) const {
		return vector3f(x - v.x, y - v.y);
	}
};

struct vector2i {
	int x, y;
	vector<vector2i> transformations;

	inline vector2i(int sx = 0, int sy = 0) {
		x = sx;
		y = sy;
	}

	inline void transform(vector2i vector)
	{
		x += vector.x;
		y += vector.y;
		transformations.push_back(vector);
	}

	inline bool operator == (const vector2i& v) const {
		return x == v.x && y == v.y;
	}

	inline vector2i operator - (const vector2i& v) const {
		return vector2i(x - v.x, y - v.y);
	}
};

struct vector3i {
	int x, y, z;
	vector<vector3i> transformations;

	vector3i(int sx = 0, int sy = 0, int sz = 0) {
		x = sx;
		y = sy;
		z = sz;
	}

	void transform(vector3i vector)
	{
		x += vector.x;
		y += vector.y;
		z += vector.z;
		transformations.push_back(vector);
	}

	bool operator == (const vector3i& v) const {
		return x == v.x && y == v.y && z == v.z;
	}

	vector3i operator - (const vector3i& v) const {
		return vector3i(x - v.x, y - v.y);
	}
};


