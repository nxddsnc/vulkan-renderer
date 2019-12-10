#include "Platform.h"
#include <vector>

const int MAX_FRAMES_IN_FLIGHT = 2;
class Window;

#pragma once
class Renderer
{
public:
	Renderer(Window *window);
	~Renderer();

	bool Run();
    void Resize(GLFWwindow*, int, int);

	VkInstance GetVulkanInstance();
	VkPhysicalDevice GetPhysicalDevice();
	VkDevice GetVulkanDevice();
	VkQueue GetVulkanDeviceQueue();
	VkPhysicalDeviceProperties GetPhycicalDeviceProperties();
	VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties();
	uint32_t GetGraphicFamilyIndex();
	VkRenderPass GetVulkanRenderPass();
	VkFramebuffer GetActiveFramebuffer();
	uint32_t GetSwapchainImageCount();
	void DrawFrame();
private:
	void _beginRender();
	void _endRender(std::vector<VkSemaphore> waitSemaphores);

	void _setupLayersAndExtensions();
	void _initInstance();
	void _deInitInstance();

	void _initDevice();
	void _deInitDevice();

	void _initGraphicsPipeline();
	void _deInitGraphicPipeline();

	void _initSurface();
	void _deInitSurface();

    void _cleanupSwapchain();

	void _initSwapchain();
	void _deInitSwapchain();

	void _initSwapchainImages();
	void _deInitSwapchainImages();

	void _initDepthStencilImage();
	void _deInitDepthStencilImage();

	void _initRenderPass();
	void _deInitRenderPass();

	void _initFramebuffers();
	void _deInitFramebuffer();

	void _initCommandBufferPool();
	void _deInitCommandBufferPool();

	void _initCommandBuffers();
	void _deInitCommandBuffers();

	void _initSynchronizations();
	void _deInitSynchronizations();

	VkShaderModule _createShaderModule(const std::vector<char>& code);

	void _setupDebug();
	void _initDebug();
	void _deInitDebug();

	VkInstance							_instance = VK_NULL_HANDLE;
	VkPhysicalDevice					_gpu = VK_NULL_HANDLE;
	VkDevice							_device = VK_NULL_HANDLE;
	VkQueue								_queue = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties			_gpuProperties;
	VkPhysicalDeviceMemoryProperties	_gpuMemoryProperties;
	uint32_t							_graphicFamilyIndex = 0;

	VkSurfaceKHR					    _surface;
	VkSurfaceCapabilitiesKHR			_surfaceCapibilities;
	std::vector<VkImage>				_swapchainImages;
	std::vector<VkImageView>			_swapchainImageViews;
	std::vector<VkFramebuffer>			_framebuffers;
	VkPipelineLayout					_pipelineLayout;
	VkPipeline							_graphicsPipeline;
	VkCommandPool						_commandPool;
	std::vector<VkCommandBuffer>		_commandBuffers;

	VkSemaphore							_imageAvailableSemaphore;
	VkSemaphore							_renderFinishedSemaphore;

	VkExtent2D							_swapchainExtent;

	VkImage								_depthStencilImage = VK_NULL_HANDLE;
	VkDeviceMemory						_depthStencilImageMemory = VK_NULL_HANDLE;
	VkImageView							_depthStencilImageView = VK_NULL_HANDLE;
	VkFormat							_depthStencilFormat = VK_FORMAT_UNDEFINED;
	boolean								_stencilAvailable = false;

	VkRenderPass						_renderPass = VK_NULL_HANDLE;

	VkSurfaceFormatKHR					_surfaceFormat;
	VkSwapchainKHR						_swapchain = VK_NULL_HANDLE;
	uint32_t							_swapchainImageCount;
	uint32_t							_activeSwapchainImageId = UINT32_MAX;

	VkFence								_swapchainImageAvailable;

	Window * _window = nullptr;

	std::vector<const char*> _instanceLayers;
	std::vector<const char*> _instanceExtensions;
	std::vector<const char*> _deviceLayers;
	std::vector<const char*> _deviceExtensions;

	VkDebugReportCallbackEXT _debugReport = VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT _debugCallbackCreateInfo = {};
};

