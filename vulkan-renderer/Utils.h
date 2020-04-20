#include "Platform.h"

struct BBox
{
	glm::vec3 min;
	glm::vec3 max;
};
#include <string>
#pragma once
std::string GetFileDirectory(std::string filename);

// https://gist.github.com/martin-kallman/5049614
void float32(float* __restrict out, const uint16_t in);

void float16(uint16_t* __restrict out, const float in);