#include "vk_mem_alloc.h"
#include "Platform.h"
#include <vector>

#pragma once


class Window;

class VulkanContext 
{
public:
    VulkanContext();
    VulkanContext(Window *window);
    ~VulkanContext();

public:
    Window                             * GetWindow();
    vk::Instance                         GetInstance();
    vk::PhysicalDevice                   GetPhysicalDevice();
    vk::Device                           GetLogicalDevice();
    vk::Queue                            GetDeviceQueue();
    VkPhysicalDeviceProperties           GetPhyscialDeivceProperties();
    vk::PhysicalDeviceMemoryProperties   GetPhysicalDeviceMemoryProperties();
    vk::SurfaceKHR                       GetSuface();
    vk::CommandPool                      GetCommandPool();
    vk::SurfaceFormatKHR                 GetSurfaceFormat();
    uint32_t                             GetGraphicsQueueFamilyIndex();

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

    void _initCommandPool();
    void _deInitCommandPool();

private:
    VkDebugReportCallbackEXT _debugReport = VK_NULL_HANDLE;
    VkDebugReportCallbackCreateInfoEXT _debugCallbackCreateInfo = {};
private:
    Window                            * _window;
    vk::Instance                        _instance = nullptr;
    vk::PhysicalDevice                    _gpu = nullptr;
    vk::Device                            _device = nullptr;
    vk::Queue                            _queue = nullptr;
    VkPhysicalDeviceProperties            _gpuProperties;
    vk::PhysicalDeviceMemoryProperties    _gpuMemoryProperties;
    uint32_t                            _graphicsQueueFamilyIndex = 0;
    vk::SurfaceKHR                        _surface;
    vk::SurfaceFormatKHR                _surfaceFormat;
    vk::CommandPool                        _commandPool;

    std::vector<const char*>            _instanceLayers;
    std::vector<const char*>            _instanceExtensions;
    std::vector<const char*>            _deviceLayers;
    std::vector<const char*>            _deviceExtensions;
};