#pragma once

#define WIDTH 1600
#define HEIGHT 1000

#ifdef _WIN32
#define USE_GLFW 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#include <windows.h>
#include <vulkan/vulkan_win32.h>

#elif __APPLE__
#define USE_GLFW 1
#define VK_USE_PLATFORM_MACOS_MVK 1
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define PLATFORM_SURFACE_EXTENSION_NAME VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#include <vulkan/vulkan_macos.h>
#include <cstdint>
typedef uint8_t  CHAR;
typedef uint16_t WORD;
typedef uint32_t DWORD;

typedef uint32_t UINT;
typedef int8_t  BYTE;
typedef int16_t SHORT;
typedef int32_t LONG;

typedef LONG INT;
typedef INT BOOL;

#define INFINITE std::numeric_limits<float>::infinity()

#else
#error Platform not supported
#endif


#include "vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vk_mem_alloc.h"


