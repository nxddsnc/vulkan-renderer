#pragma once
#include "Platform.h"
#include <vector>
#include <string>

class VulkanRenderer;
struct GLFWwindow;

class Window
{
public:
	Window(uint32_t sizeX, uint32_t sizeY, std::string name);
	~Window();

	void Close();
	bool Update();
	void Resize(int width, int height);

	void InitOSSurface(vk::Instance instance, VkSurfaceKHR *surface);
	vk::Extent2D GetWindowExtent();
    void SetWindowExtent(VkExtent2D extent);
    GLFWwindow *GetGLFWWindow();
private:
	bool windowRunning;

private:
	void _initOSWindow();
	void _deInitOSWindow();
	void _updateOSWindow();

	VulkanRenderer                        *   _renderer;
	uint32_t							_width = 512;
	uint32_t							_height = 512;
	std::string							_windowName;

#ifdef USE_GLFW
    GLFWwindow                      *   _window;
#elif VK_USE_PLATFORM_WIN32_KHR
	HINSTANCE							_win32Instance = NULL;
	HWND								_win32Window = NULL;
	std::string							_win32ClassName;
	static uint64_t						_win32ClassIdCounter;
#elif VK_USE_PLATFORM_XCB_KHR
	xcb_connection_t				*	_xcb_connection = nullptr;
	xcb_screen_t					*	_xcb_screen = nullptr;
	xcb_window_t						_xcb_window = 0;
	xcb_in
#endif
};

