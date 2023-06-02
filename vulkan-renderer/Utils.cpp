#include "Utils.h"
#include <algorithm>
#include <cctype>
#include <string>

std::string GetFileDirectory(std::string filename)
{
	std::string directory;
	const size_t last_slash_idx = filename.rfind('/');
	if (std::string::npos != last_slash_idx)
	{
		directory = filename.substr(0, last_slash_idx);
	}
	return directory;
}

// https://gist.github.com/martin-kallman/5049614
// float32
// Martin Kallman
//
// Fast half-precision to single-precision floating point conversion
//  - Supports signed zero and denormals-as-zero (DAZ)
//  - Does not support infinities or NaN
//  - Few, partially pipelinable, non-branching instructions,
//  - Core opreations ~6 clock cycles on modern x86-64
void float32(float* __restrict out, const uint16_t in) {
	uint32_t t1;
	uint32_t t2;
	uint32_t t3;

	t1 = in & 0x7fff;                       // Non-sign bits
	t2 = in & 0x8000;                       // Sign bit
	t3 = in & 0x7c00;                       // Exponent

	t1 <<= 13;                              // Align mantissa on MSB
	t2 <<= 16;                              // Shift sign bit into position

	t1 += 0x38000000;                       // Adjust bias

	t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

	t1 |= t2;                               // Re-insert sign bit

	*((uint32_t*)out) = t1;
};

// float16
// Martin Kallman
//
// Fast single-precision to half-precision floating point conversion
//  - Supports signed zero, denormals-as-zero (DAZ), flush-to-zero (FTZ),
//    clamp-to-max
//  - Does not support infinities or NaN
//  - Few, partially pipelinable, non-branching instructions,
//  - Core opreations ~10 clock cycles on modern x86-64
void float16(uint16_t* __restrict out, const float in) {
	uint32_t inu = *((uint32_t*)&in);
	uint32_t t1;
	uint32_t t2;
	uint32_t t3;

	t1 = inu & 0x7fffffff;                 // Non-sign bits
	t2 = inu & 0x80000000;                 // Sign bit
	t3 = inu & 0x7f800000;                 // Exponent

	t1 >>= 13;                             // Align mantissa on MSB
	t2 >>= 16;                             // Shift sign bit into position

	t1 -= 0x1c000;                         // Adjust bias

	t1 = (t3 > 0x38800000) ? 0 : t1;       // Flush-to-zero
	t1 = (t3 < 0x8e000000) ? 0x7bff : t1;  // Clamp-to-max
	t1 = (t3 == 0 ? 0 : t1);               // Denormals-as-zero

	t1 |= t2;                              // Re-insert sign bit

	*((uint16_t*)out) = t1;
}

void ApplyMatrixToBBox(glm::mat4 & mat, BBox & bbox)
{
	glm::vec3 points[7] = { glm::vec3(bbox.min.x, bbox.max.y, bbox.max.z), glm::vec3(bbox.max.x, bbox.min.y, bbox.max.z), glm::vec3(bbox.max.x, bbox.max.y, bbox.min.z),
							glm::vec3(bbox.min.x, bbox.min.y, bbox.max.z), glm::vec3(bbox.min.x, bbox.max.y, bbox.min.z), glm::vec3(bbox.max.x, bbox.min.y, bbox.min.z), 
							bbox.min };
	bbox.min = mat * glm::vec4(bbox.max, 1.0);

	for (int i = 0; i < 7; ++i)
	{
		points[i] = mat * glm::vec4(points[i], 1.0);
		if (bbox.min.x > points[i].x)
		{
			bbox.min.x = points[i].x;
		}
		if (bbox.min.y > points[i].y)
		{
			bbox.min.y = points[i].y;
		}
		if (bbox.min.z > points[i].z)
		{
			bbox.min.z = points[i].z;
		}
		if (bbox.max.x < points[i].x)
		{
			bbox.max.x = points[i].x;
		}
		if (bbox.max.y < points[i].y)
		{
			bbox.max.y = points[i].y;
		}
		if (bbox.max.z < points[i].z)
		{
			bbox.max.z = points[i].z;
		}
	}
}

bool HasEnding(std::string const& fullString, std::string const& ending)
{
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}