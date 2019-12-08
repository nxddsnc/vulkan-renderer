#include "Renderer.h"
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include "BUILD_OPTIONS.h"
#include "Shared.h"
#include "Window.h"
#include <array>

Renderer::Renderer(Window *window)
{
	_window = window;
	_setupLayersAndExtensions();
	_setupDebug();
	_initInstance();
	_initDebug();
	_initDevice();
	_initSurface();
	_initSwapchain();
	_initSwapchainImages();
	_initDepthStencilImage();
	_initRenderPass();
	_initGraphicsPipeline();
	_initFramebuffers();
	_initCommandBufferPool();
	_initCommandBuffers();
	_initSynchronizations();
}

Renderer::~Renderer()
{
	vkQueueWaitIdle(_queue);
	delete _window;
	_deInitSynchronizations();
	_deInitCommandBuffers();
	_deInitCommandBufferPool();
	_deInitFramebuffer();
	_initGraphicsPipeline();
	_deInitRenderPass();
	_deInitDepthStencilImage();
	_deInitSwapchainImages();
	_deInitSwapchain();
	_deInitSurface();
	_deInitDevice();
	_deInitDebug();
	_deInitInstance();
}

bool Renderer::Run()
{
	if (nullptr != _window) 
	{
		return _window->Update();
	}
	return true;
}

VkInstance Renderer::GetVulkanInstance()
{
	return _instance;
}

VkPhysicalDevice Renderer::GetPhysicalDevice()
{
	return _gpu;
}

VkDevice Renderer::GetVulkanDevice()
{
	return _device;
}

VkQueue Renderer::GetVulkanDeviceQueue()
{
	return _queue;
}

VkPhysicalDeviceProperties Renderer::GetPhycicalDeviceProperties()
{
	return _gpuProperties;
}

VkPhysicalDeviceMemoryProperties Renderer::GetPhysicalDeviceMemoryProperties()
{
	return _gpuMemoryProperties;
}

uint32_t Renderer::GetGraphicFamilyIndex()
{
	return _graphicFamilyIndex;
}

VkRenderPass Renderer::GetVulkanRenderPass()
{
	return _renderPass;
}

VkFramebuffer Renderer::GetActiveFramebuffer()
{
	return _framebuffers[_activeSwapchainImageId];
}

uint32_t Renderer::GetSwapchainImageCount()
{
	return uint32_t();
}

void Renderer::BeginRender()
{
	vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, 0, _swapchainImageAvailable, &_activeSwapchainImageId);
	vkWaitForFences(_device, 1, &_swapchainImageAvailable, VK_TRUE, UINT64_MAX);
	vkResetFences(_device, 1, &_swapchainImageAvailable);
	vkQueueWaitIdle(_queue);
}

void Renderer::EndRender(std::vector<VkSemaphore> waitSemaphores)
{
	VkResult  presentResult = VkResult::VK_RESULT_MAX_ENUM;
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = waitSemaphores.size();
	presentInfo.pWaitSemaphores = waitSemaphores.data();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain;
	presentInfo.pImageIndices = &_activeSwapchainImageId;
	presentInfo.pResults = &presentResult;
	ErrorCheck(vkQueuePresentKHR(_queue, &presentInfo));
}

void Renderer::_setupLayersAndExtensions()
{
	_instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME );
	_instanceExtensions.push_back(PLATFORM_SURFACE_EXTENSION_NAME);

	_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void Renderer::_initInstance()
{
	VkApplicationInfo applicationInfo;
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "Vulkan Go";
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = &_debugCallbackCreateInfo;
	instanceCreateInfo.enabledLayerCount = _instanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = _instanceLayers.data();
	instanceCreateInfo.enabledExtensionCount = _instanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = _instanceExtensions.data();

	ErrorCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));
}

void Renderer::_deInitInstance()
{
	vkDestroyInstance(_instance, nullptr);
	_instance = nullptr;
}

void Renderer::_initDevice()
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

void Renderer::_deInitDevice()
{
	vkDestroyDevice(_device, nullptr);
	return;
}

void Renderer::_initGraphicsPipeline()
{
	auto vertShaderCode = readFile("Shaders/basic_vert.spv");
	auto fragShaderCode = readFile("Shaders/basic_frag.spv");
	VkShaderModule vertShaderModule = _createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = _createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	// Input Assemble
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewports and Sissors
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	// FIXME: The viewport size should be read from swapchain image size.
	viewport.width = _swapchainExtent.width;
	viewport.height = _swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = _swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// Depth and stencil testing
	// TODO: To be done

	// Color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
	ErrorCheck(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout));

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	ErrorCheck(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline));

	vkDestroyShaderModule(_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}

void Renderer::_deInitGraphicPipeline()
{
	vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
}


void Renderer::_initSurface()
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

void Renderer::_deInitSurface()
{
	vkDestroySurfaceKHR(_instance, _surface, nullptr);
}

VkShaderModule Renderer::_createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkShaderModule shaderModule;
	ErrorCheck(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule));
	return shaderModule;
}

#ifdef BUILD_ENABLE_VULKAN_DEBUG

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
void Renderer::_setupDebug()
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

PFN_vkCreateDebugReportCallbackEXT  fvkCreateDebugReportCallbackExt = nullptr;
PFN_vkDestroyDebugReportCallbackEXT  fvkDestroyDebugReportCallbackExt = nullptr;

void Renderer::_initDebug()
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

void Renderer::_deInitDebug()
{
	fvkDestroyDebugReportCallbackExt(_instance, _debugReport, nullptr);
	_debugReport = nullptr;
}

void Renderer::_initSwapchain()
{
	if (_swapchainImageCount > _surfaceCapibilities.maxImageCount)
	{
		_swapchainImageCount = _surfaceCapibilities.maxImageCount;
	}
	if (_swapchainImageCount < _surfaceCapibilities.minImageCount)
	{
		_swapchainImageCount = _surfaceCapibilities.minImageCount;
	}

	_swapchainExtent = { WIDTH, HEIGHT };

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpu, _surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpu, _surface, &presentModeCount, presentModes.data());
		for (auto m : presentModes)
		{
			if (m == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = m;
				break;
			}
		}
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _surface;
	createInfo.minImageCount = _swapchainImageCount;
	createInfo.imageFormat = _surfaceFormat.format;
	createInfo.imageColorSpace = _surfaceFormat.colorSpace;
	createInfo.imageExtent = _window->GetWindowExtent();
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	ErrorCheck(vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain));

	ErrorCheck(vkGetSwapchainImagesKHR(_device, _swapchain, &_swapchainImageCount, nullptr));
}
void Renderer::_deInitSwapchain()
{
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}
void Renderer::_initSwapchainImages()
{
	_swapchainImages.resize(_swapchainImageCount);
	_swapchainImageViews.resize(_swapchainImageCount);

	ErrorCheck(vkGetSwapchainImagesKHR(_device, _swapchain, &_swapchainImageCount, _swapchainImages.data()));

	for (uint32_t i = 0; i < _swapchainImageCount; ++i)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = _swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = _surfaceFormat.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		ErrorCheck(vkCreateImageView(_device, &createInfo, nullptr, &_swapchainImageViews[i]));
	}
}
void Renderer::_deInitSwapchainImages()
{
	for (uint32_t i = 0; i < _swapchainImageCount; ++i)
	{
		vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
	}
}

void Renderer::_initDepthStencilImage()
{
	{
		std::vector<VkFormat> tryFormats{
			VK_FORMAT_D24_UNORM_S8_UINT ,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM
		};
		for (auto f : tryFormats)
		{
			VkFormatProperties formatProperties = {};
			vkGetPhysicalDeviceFormatProperties(_gpu, f, &formatProperties);
			if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				_depthStencilFormat = f;
				break;
			}
		}
		if (_depthStencilFormat == VK_FORMAT_UNDEFINED)
		{
			assert(1 && "wrong format found!");
			exit(-1);
		}
		if (_depthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
			_depthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
			_depthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
			_depthStencilFormat == VK_FORMAT_S8_UINT)
		{
			_stencilAvailable = true;
		}
	}

	VkImageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.format = _depthStencilFormat;
	createInfo.extent.width = _swapchainExtent.width;
	createInfo.extent.height = _swapchainExtent.height;
	createInfo.extent.depth = 1;
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkCreateImage(_device, &createInfo, nullptr, &_depthStencilImage);

	VkMemoryRequirements imageMemoryRequirements;
	vkGetImageMemoryRequirements(_device, _depthStencilImage, &imageMemoryRequirements);
	auto requiredProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	uint32_t memoryIndex = FindMemoryTypeIndex(&_gpuMemoryProperties, &imageMemoryRequirements, requiredProperties);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = imageMemoryRequirements.size;
	allocateInfo.memoryTypeIndex = memoryIndex;
	ErrorCheck(vkAllocateMemory(_device, &allocateInfo, nullptr, &_depthStencilImageMemory));
	vkBindImageMemory(_device, _depthStencilImage, _depthStencilImageMemory, 0);

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = _depthStencilImage;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = _depthStencilFormat;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (_stencilAvailable ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &_depthStencilImageView);
}

void Renderer::_deInitDepthStencilImage()
{
	vkDestroyImageView(_device, _depthStencilImageView, nullptr);
	vkFreeMemory(_device, _depthStencilImageMemory, nullptr);
	vkDestroyImage(_device, _depthStencilImage, nullptr);
}

void Renderer::_initRenderPass()
{
	std::array<VkAttachmentDescription, 2> attachments{};

	attachments[0].flags = 0;
	attachments[0].format = _depthStencilFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachments[1].flags = 0;
	attachments[1].format = _surfaceFormat.format;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference subpassDepthStencilAttachment = {};
	subpassDepthStencilAttachment.attachment = 0;
	subpassDepthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentReference, 1> subpassColorAttachments{};
	subpassColorAttachments[0].attachment = 1;
	subpassColorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::array<VkSubpassDescription, 1> subpasses{};
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].inputAttachmentCount = 0;
	subpasses[0].pInputAttachments = VK_NULL_HANDLE;
	subpasses[0].colorAttachmentCount = subpassColorAttachments.size();
	subpasses[0].pColorAttachments = subpassColorAttachments.data();
	subpasses[0].pDepthStencilAttachment = &subpassDepthStencilAttachment;

	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = subpasses.size();
	createInfo.pSubpasses = subpasses.data();

	ErrorCheck(vkCreateRenderPass(_device, &createInfo, nullptr, &_renderPass));
}

void Renderer::_deInitRenderPass()
{
	vkDestroyRenderPass(_device, _renderPass, nullptr);
}

void Renderer::_initFramebuffers()
{
	_framebuffers.resize(_swapchainImageCount);
	for (uint32_t i = 0; i < _swapchainImageCount; ++i)
	{
		std::array<VkImageView, 2> attachments{};
		attachments[0] = _depthStencilImageView;
		attachments[1] = _swapchainImageViews[i];

		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = _renderPass;
		createInfo.attachmentCount = attachments.size();
		createInfo.pAttachments = attachments.data();
		createInfo.width = _swapchainExtent.width;
		createInfo.height = _swapchainExtent.height;
		createInfo.layers = 1;
		ErrorCheck(vkCreateFramebuffer(_device, &createInfo, nullptr, &_framebuffers[i]));
	}
}

void Renderer::_deInitFramebuffer()
{
	for (uint32_t i = 0; i < _swapchainImageCount; ++i)
	{
		vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
	}
}

void Renderer::_initCommandBufferPool()
{
}

void Renderer::_deInitCommandBufferPool()
{
}

void Renderer::_initCommandBuffers()
{
}

void Renderer::_deInitCommandBuffers()
{
}

void Renderer::_initSynchronizations()
{
	VkFenceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(_device, &createInfo, nullptr, &_swapchainImageAvailable);
}

void Renderer::_deInitSynchronizations()
{
	vkDestroyFence(_device, _swapchainImageAvailable, nullptr);
}


#else

void Renderer::_setupDebug() {}
void Renderer::_initDebug() {}
void Renderer::_deInitDebug() {}

#endif // BUILLD_ENABLED_VULKAN_DEBUG