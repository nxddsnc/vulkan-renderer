#pragma once

#define WIDTH 800
#define HEIGHT 600

#ifdef _WIN32 
#define USE_GLFW 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#include <windows.h>

#elif
#error Platform not supported
#endif

#include "vulkan.hpp"
#include <glm/glm.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"