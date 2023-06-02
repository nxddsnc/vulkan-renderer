#pragma once

#define WIDTH 800
#define HEIGHT 600
#define PREFILTERED_IMAGE_WIDTH 512
#define PREFILTERED_IMAGE_HEIGHT 512

#ifdef _WIN32 
#define USE_GLFW 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#include <windows.h>
#include <vulkan/vulkan_win32.h>

#elif
#error Platform not supported
#endif


#include "vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vk_mem_alloc.h"


