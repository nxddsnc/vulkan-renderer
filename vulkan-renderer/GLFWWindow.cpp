#include "BUILD_OPTIONS.h"
#include "Platform.h"
#include "Window.h"
#include <assert.h>
#include "VulkanRenderer.h"
#include "Shared.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
#ifdef USE_GLFW
void Window::_initOSWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);        // This tells GLFW to not create an OpenGL context with the window
    _window = glfwCreateWindow(_width, _height, _windowName.c_str(), nullptr, nullptr);
    
    // make sure we indeed get the surface size we want.
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    _width = width;
    _height = height;
}

void Window::_deInitOSWindow()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void Window::_updateOSWindow()
{

}

void Window::InitOSSurface(vk::Instance instance, VkSurfaceKHR *surface)
{
    glfwCreateWindowSurface(instance, _window, nullptr, surface);
}

#endif
#endif