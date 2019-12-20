#include "Platform.h"

#pragma once
class Window;

class VulkanContext 
{
public:
    VulkanContext(Window *window);
    ~VulkanContext();

private: 
    void _initVulkan();
    void _deInitVulkan();

    void _setupLayersAndExtensions();
	void _setupDebug();
	void _initDebug();
	void _deInitDebug();

    void _initInstance();
	void _deInitInstance();

	void _initDevice();
	void _deInitDevice();

	void _initSurface();
	void _deInitSurface();

	void findQueueFamilyIndices();
	void createCommandPools();

private:
    VkDebugReportCallbackEXT _debugReport = VK_NULL_HANDLE;
    VkDebugReportCallbackCreateInfoEXT _debugCallbackCreateInfo = {};
private:
    Window                            * _window;
    vk::Instance                        _instance = nullptr;
	vk::PhysicalDevice					_gpu = nullptr;
	vk::Device							_device = nullptr;
	vk::Queue							_queue = nullptr;
	VkPhysicalDeviceProperties		    _gpuProperties;
	vk::PhysicalDeviceMemoryProperties	_gpuMemoryProperties;
	uint32_t							_graphicFamilyIndex = 0;
	vk::SurfaceKHR					    _surface;

    std::vector<const char*>            _instanceLayers;
    std::vector<const char*>            _instanceExtensions;
    std::vector<const char*>            _deviceLayers;
    std::vector<const char*>            _deviceExtensions;
}