#include "Context.h"
#include <iostream>
#include "Shared.h"
#include "Window.h"

VulkanContext::VulkanContext()
{

}

VulkanContext::VulkanContext(Window *window)
{
    _window = window;
    _initVulkan();
}   

VulkanContext::~VulkanContext()
{
    _deInitVulkan();
}

Window * VulkanContext::GetWindow()
{
    return _window;
}

vk::Instance VulkanContext::GetInstance()
{
    return _instance;
}

vk::PhysicalDevice VulkanContext::GetPhysicalDevice()
{
    return _gpu;
}

vk::Device VulkanContext::GetLogicalDevice()
{
    return _device;
}

vk::Queue VulkanContext::GetDeviceQueue()
{
    return _queue;
}

VkPhysicalDeviceProperties VulkanContext::GetPhyscialDeivceProperties()
{
    return _gpuProperties;
}

vk::PhysicalDeviceMemoryProperties VulkanContext::GetPhysicalDeviceMemoryProperties()
{
    return _gpuMemoryProperties;
}

vk::SurfaceKHR VulkanContext::GetSurface()
{
    return _surface;
}

vk::CommandPool VulkanContext::GetCommandPool()
{
    return _commandPool;
}

vk::SurfaceFormatKHR VulkanContext::GetSurfaceFormat()
{
    return _surfaceFormat;
}

uint32_t VulkanContext::GetGraphicsQueueFamilyIndex()
{
    return _graphicsQueueFamilyIndex;
}

void VulkanContext::_setupLayersAndExtensions()
{
    _instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME );
    _instanceExtensions.push_back(PLATFORM_SURFACE_EXTENSION_NAME);

    _deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
    VkDebugReportFlagsEXT msgFlags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t sourceObject,
    size_t location,
    int32_t msgCode,
    const char* layerPrefix,
    const char* message,
    void* useData
)
{
    if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
        std::cout << "info: " << std::endl;
    }
    if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        std::cout << "warning: " << std::endl;
    }
    if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        std::cout << "performance warning: " << std::endl;
    }
    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        std::cout << "error: " << std::endl;
    }
    std::cout << "[" << layerPrefix << "]" << std::endl;
    std::cout << message << std::endl;
    return false;
}

void VulkanContext::_setupDebug()
{
    _debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    _debugCallbackCreateInfo.pfnCallback = VulkanDebugCallback;
    _debugCallbackCreateInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT |
        VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT |
        0;

    //_instanceLayers.push_back()
    _instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
    
#if __APPLE__
    _instanceExtensions.push_back("VK_EXT_metal_surface");
#endif

    _instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    //VK_LAYER_NV_optimus
    //VK_LAYER_RENDERDOC_Capture
    //VK_LAYER_VALVE_steam_overlay
    //VK_LAYER_VALVE_steam_fossilize
    //VK_LAYER_LUNARG_api_dump
    //VK_LAYER_LUNARG_device_simulation
    //VK_LAYER_KHRONOS_validation
    //VK_LAYER_LUNARG_monitor
    //VK_LAYER_LUNARG_screenshot
    //VK_LAYER_LUNARG_standard_validation
    //VK_LAYER_LUNARG_vktrace

    _deviceLayers.push_back("VK_LAYER_KHRONOS_validation");
}


void VulkanContext::_initVulkan()
{
    _setupLayersAndExtensions();
    _setupDebug();
    _initInstance();
    _initDebug();
    _initDevice();
    _initSurface();
    _initCommandPool();
}

void VulkanContext::_deInitVulkan()
{
    _deInitCommandPool();
    _deInitSurface();
    _deInitDevice();
    _deInitDebug();
    _deInitInstance();
}

void VulkanContext::_initInstance()
{
    vk::ApplicationInfo applicationInfo({
        "Vulkan Renderer",
        1,
        "Vulkan",
        VK_API_VERSION_1_1,
        VK_API_VERSION_1_1
    });

    vk::InstanceCreateInfo instanceCreateInfo({
        {},
        &applicationInfo,
        (uint32_t)_instanceLayers.size(),
        _instanceLayers.data(),
        (uint32_t)_instanceExtensions.size(),
        _instanceExtensions.data()
    });
    instanceCreateInfo.pNext = &_debugCallbackCreateInfo;

    ErrorCheck(vk::createInstance(&instanceCreateInfo, nullptr, &_instance));
}

void VulkanContext::_deInitInstance()
{
    _instance.destroy();
    _instance = nullptr;
}

PFN_vkCreateDebugReportCallbackEXT  fvkCreateDebugReportCallbackExt = nullptr;
PFN_vkDestroyDebugReportCallbackEXT  fvkDestroyDebugReportCallbackExt = nullptr;

void VulkanContext::_initDebug()
{
    fvkCreateDebugReportCallbackExt = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
    fvkDestroyDebugReportCallbackExt = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");

    if (fvkCreateDebugReportCallbackExt == nullptr)
    {
        assert(1 && "VULKAN ERROR: can't fetch debug function pointers.");
        return;
    }

    if (fvkDestroyDebugReportCallbackExt == nullptr)
    {
        assert(1 && "VULKAN ERROR: can't fetch debug function pointers.");
        return;
    }

    fvkCreateDebugReportCallbackExt(_instance, &_debugCallbackCreateInfo, nullptr, &_debugReport);
}

void VulkanContext::_deInitDebug()
{
    fvkDestroyDebugReportCallbackExt(_instance, _debugReport, nullptr);
    _debugReport = VK_NULL_HANDLE;
}

void VulkanContext::_initDevice()
{
    {
        uint32_t gpuCount = 0;
        std::vector<vk::PhysicalDevice> gpuList = _instance.enumeratePhysicalDevices();
        _gpu = gpuList[0];
        _gpuProperties = _gpu.getProperties();
        _gpuMemoryProperties = _gpu.getMemoryProperties();
    }

    {
        std::vector<vk::QueueFamilyProperties> familyPropertyList = _gpu.getQueueFamilyProperties();
        bool found = false;
        for (uint32_t i = 0; i < familyPropertyList.size(); ++i)
        {
            if (familyPropertyList[i].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                found = true;
                _graphicsQueueFamilyIndex = i;
                break;
            }
        }
        if (!found)
        {
            assert(1 && "Vulkan Error: Queue family supporting graphics not found!");
            std::exit(1);
        }
    }

    {
        std::vector<vk::LayerProperties> layerProperties = vk::enumerateInstanceLayerProperties();
        std::cout << "instance layers: \n";
        for (auto &i : layerProperties)
        {
            std::cout << "    " << "layer name: " <<  i.layerName << "\t\t" << "layer descrption: " << i.description << std::endl;
        }
        std::cout << std::endl;
    }

    float quePriorities[] = { 1.0f };
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo({
        {},
        _graphicsQueueFamilyIndex,
        1,
        quePriorities
    });

    vk::PhysicalDeviceFeatures physicalDeviceFeatures = {};
    physicalDeviceFeatures.samplerAnisotropy = vk::Bool32(true);
    vk::DeviceCreateInfo deviceCreateInfo({
        {},
        1,
        &deviceQueueCreateInfo,
        (uint32_t)_deviceLayers.size(),
        _deviceLayers.data(),
        (uint32_t)_deviceExtensions.size(),
        _deviceExtensions.data(),
        &physicalDeviceFeatures
    });
    physicalDeviceFeatures.fillModeNonSolid = true;

    _device = _gpu.createDevice(deviceCreateInfo);
    _queue = _device.getQueue(_graphicsQueueFamilyIndex, 0);
}

void VulkanContext::_deInitDevice()
{
    _device.destroy();
    return;
}

void VulkanContext::_initSurface()
{
    VkSurfaceKHR temp_surface;
    _window->InitOSSurface(_instance, &temp_surface);
    _surface = vk::SurfaceKHR(temp_surface);

    VkBool32 isSupported = _gpu.getSurfaceSupportKHR(_graphicsQueueFamilyIndex, _surface);
    if (!isSupported)
    {
        assert(1 && "WSI not supported.");
        exit(-1);
    }

    vk::SurfaceCapabilitiesKHR surfaceCapibilities = _gpu.getSurfaceCapabilitiesKHR(_surface);
    if (surfaceCapibilities.currentExtent.width < UINT32_MAX)
    {

    }
    //if (_surfaceCapibilities.currentExtent.width < UINT32_MAX)
    //{
    //    //_surface_size_x = _surfaceCapibilities.currentExtent.width;
    //    //_surface_size_y = _surfaceCapibilities.currentExtent.height;
    //}

    {
        std::vector<vk::SurfaceFormatKHR> surfaceFormats = _gpu.getSurfaceFormatsKHR(_surface);
        if (surfaceFormats.size() == 0)
        {
            assert(1 && "Surface format missing.");
            exit(-1);
        }
        if (surfaceFormats[0].format == vk::Format::eUndefined)
        {
            _surfaceFormat = vk::SurfaceFormatKHR({
                vk::Format::eB8G8R8Unorm,
                vk::ColorSpaceKHR::eSrgbNonlinear
            });
        }
        else
        {
            _surfaceFormat = surfaceFormats[0];
        }
    }
}

void VulkanContext::_deInitSurface()
{
    _instance.destroySurfaceKHR(_surface);
}

void VulkanContext::_initCommandPool()
{
    vk::CommandPoolCreateInfo commandPoolCreateInfo({
        vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        _graphicsQueueFamilyIndex
    });
    _commandPool = _device.createCommandPool(commandPoolCreateInfo);
}

void VulkanContext::_deInitCommandPool()
{
    _device.destroyCommandPool(_commandPool);
}

