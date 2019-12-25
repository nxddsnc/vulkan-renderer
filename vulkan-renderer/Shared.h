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
void ErrorCheck(vk::Result result);
#endif

std::vector<char> readFile(const std::string& filename);

uint32_t FindMemoryTypeIndex(VkPhysicalDeviceMemoryProperties *gpuMemoryProperties, 
	const VkMemoryRequirements *momoryRequiremenets, const VkMemoryPropertyFlags memoryProperties);

/************************************************************************/
/*                                                                      */
/************************************************************************/

void createImage(vk::Device &device, vk::PhysicalDeviceMemoryProperties * gpuMemoryProperties,
	uint32_t width, uint32_t height, VkFormat format,
	vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
	vk::Image& image, vk::DeviceMemory& imageMemory);