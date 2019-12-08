#include "Window.h"
#include "Renderer.h"
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

VkExtent2D Window::GetWindowExtent()
{
	return{ _width, _height };
}
