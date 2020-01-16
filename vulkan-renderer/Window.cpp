#include "Window.h"
#include "Shared.h"
#include <array>
Window::Window(uint32_t sizeX, uint32_t sizeY, std::string name)
{
	_width = sizeX;
	_height = sizeY;
	_windowName = name;
	_initOSWindow();

	windowRunning = true;
}

Window::~Window()
{
	_deInitOSWindow();
}

void Window::Close()
{
	windowRunning = false;
}

bool Window::Update()
{
	_updateOSWindow();
	return windowRunning;
}

void Window::Resize(int width, int height)
{
	_width = width;
	_height = height;
}

vk::Extent2D Window::GetWindowExtent()
{
	return vk::Extent2D({ _width, _height });
}

void Window::SetWindowExtent(VkExtent2D extent)
{
    _width = extent.width;
    _height = extent.height;
}

GLFWwindow * Window::GetGLFWWindow()
{
    return _window;
}
