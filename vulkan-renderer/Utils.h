#include "Platform.h"

class BBox
{
public:
	glm::vec3 min;
	glm::vec3 max;

	void Merge(BBox& bbox)
	{

		if (min.x > bbox.min.x)
		{
			min.x = bbox.min.x;
		}
		if (min.y > bbox.min.y)
		{
			min.y = bbox.min.y;
		}
		if (min.z > bbox.min.z)
		{
			min.z = bbox.min.z;
		}

		if (max.x < bbox.max.x)
		{
			max.x = bbox.max.x;
		}
		if (max.y < bbox.max.y)
		{
			max.y = bbox.max.y;
		}
		if (max.z < bbox.max.z)
		{
			max.z = bbox.max.z;
		}
	}
};
#include <string>
#pragma once
std::string GetFileDirectory(std::string filename);

// https://gist.github.com/martin-kallman/5049614
void float32(float* __restrict out, const uint16_t in);

void float16(uint16_t* __restrict out, const float in);

void ApplyMatrixToBBox(glm::mat4& mat, BBox& bbox);

template <class T>
inline void hash_combine(std::size_t & s, const T & v)
{
    std::hash<T> h;
    s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

bool HasEnding(std::string const& fullString, std::string const& ending);