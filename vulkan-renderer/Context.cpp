#include "Context.h"
#include <iostream>
#include "Shared.h"

VulkanContext::VulkanContext(Window *window)
{
    _window = window;
    _initVulkan();
}   

VulkanContext::~VulkanContext()
{
    _deInitVulkan();
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
	_instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");

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

	_deviceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
}


void VulkanContext::_initVulkan()
{
    _setupLayersAndExtensions();
	_setupDebug();
    _initInstance();
	_initDebug();
	_initDevice();
}

void VulkanContext::_deInitVulkan()
{
    _deInitDevice();
    _deInitDebug();
    _deInitInstance();
}

void VulkanContext::_initInstance()
{
	vk::ApplicationInfo applicationInfo;
	applicationInfo.pApplicationName = "Vulkan Go";
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	vk::InstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.pNext = &_debugCallbackCreateInfo;
	instanceCreateInfo.enabledLayerCount = _instanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = _instanceLayers.data();
	instanceCreateInfo.enabledExtensionCount = _instanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = _instanceExtensions.data();

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
	_debugReport = nullptr;
}

void VulkanContext::_initDevice()
{
	{
		uint32_t gpuCount = 0;
		vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr);
		std::vector<VkPhysicalDevice> gpuList(gpuCount);
		vkEnumeratePhysicalDevices(_instance, &gpuCount, gpuList.data());
		_gpu = gpuList[0];
		vkGetPhysicalDeviceProperties(_gpu, &_gpuProperties);
		vkGetPhysicalDeviceMemoryProperties(_gpu, &_gpuMemoryProperties);
	}

	{
		uint32_t familyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &familyCount, nullptr);
		std::vector<VkQueueFamilyProperties> familyPropertyList(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &familyCount, familyPropertyList.data());
		bool found = false;
		for (uint32_t i = 0; i < familyCount; ++i)
		{
			if (familyPropertyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				found = true;
				_graphicFamilyIndex = i;
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
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> layerProperties(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
		std::cout << "instance layers: \n";
		for (auto &i : layerProperties)
		{
			std::cout << "    " << "layer name: " <<  i.layerName << "\t\t" << "layer descrption: " << i.description << std::endl;
		}
		std::cout << std::endl;
	}

	{
		uint32_t layerCount;
		vkEnumerateDeviceLayerProperties(_gpu, &layerCount, nullptr);
		std::vector<VkLayerProperties> layerProperties(layerCount);
		vkEnumerateDeviceLayerProperties(_gpu, &layerCount, layerProperties.data());
		std::cout << "device layers: \n";
		for (auto &i : layerProperties)
		{
			std::cout << "    " << "layer name: " << i.layerName << "\t\t" << "layer descrption: " << i.description << std::endl;
		}
		std::cout << std::endl;
	}

	float quePriorities[] = { 1.0f };
	VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex = _graphicFamilyIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = quePriorities;

	VkDeviceCreateInfo	deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = _deviceLayers.size();
	deviceCreateInfo.ppEnabledLayerNames = _deviceLayers.data();
	deviceCreateInfo.enabledExtensionCount = _deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = _deviceExtensions.data();


	ErrorCheck(vkCreateDevice(_gpu, &deviceCreateInfo, nullptr, &_device));

	vkGetDeviceQueue(_device, _graphicFamilyIndex, 0, &_queue);
}

void VulkanContext::_deInitDevice()
{
	vkDestroyDevice(_device, nullptr);
	return;
}

void VulkanContext::_initSurface()
{
	_window->InitOSSurface(_instance, &_surface);

	VkBool32 isSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(_gpu, _graphicFamilyIndex , _surface, &isSupported);
	if (!isSupported)
	{
		assert(1 && "WSI not supported.");
		exit(-1);
	}
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gpu, _surface, &_surfaceCapibilities);
	if (_surfaceCapibilities.currentExtent.width < UINT32_MAX)
	{
		//_surface_size_x = _surfaceCapibilities.currentExtent.width;
		//_surface_size_y = _surfaceCapibilities.currentExtent.height;
	}

	{
		uint32_t surfaceFormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(_gpu, _surface, &surfaceFormatCount, VK_NULL_HANDLE);
		if (surfaceFormatCount == 0)
		{
			assert(1 && "Surface format missing.");
			exit(-1);
		}
		std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_gpu, _surface, &surfaceFormatCount, surfaceFormats.data());
		if (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			_surfaceFormat.format = VK_FORMAT_B8G8R8_UNORM;
			_surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		}
		else
		{
			_surfaceFormat = surfaceFormats[0];
		}
	}
}

void VulkanContext::_deInitSurface()
{
	vkDestroySurfaceKHR(_instance, _surface, nullptr);
}