/* -----------------------------------------------------
This source code is public domain ( CC0 )
The code is provided as-is without limitations, requirements and responsibilities.
Creators and contributors to this source code are provided as a token of appreciation
and no one associated with this source code can be held responsible for any possible
damages or losses of any kind.
Original file creator:  Niko Kauppi (Code maintenance)
Contributors:
----------------------------------------------------- */

#pragma once

#include <iostream>
#include <assert.h>
#include "Platform.h"
#include "BUILD_OPTIONS.h"
#include <vector>

#ifdef BUILD_ENABLE_VULKAN_RUNTIME_DEBUG
void ErrorCheck(VkResult result);
#endif

uint32_t FindMemoryTypeIndex(VkPhysicalDeviceMemoryProperties *gpuMemoryProperties, const VkMemoryRequirements *momoryRequiremenets, const VkMemoryPropertyFlags memoryProperties);

std::vector<char> readFile(const std::string& filename);
