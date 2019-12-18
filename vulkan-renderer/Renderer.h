#include "Platform.h"
#include <vector>

const int MAX_FRAMES_IN_FLIGHT = 2;
class Window;

#pragma once

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class Renderer
{
public:
	Renderer(Window *window);
	~Renderer();

	bool Run();
    void Resize(int width, int height);

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
	void _updateUniformBuffer();

	void _setupLayersAndExtensions();
	void _initInstance();
	void _deInitInstance();

	void _initDevice();
	void _deInitDevice();

	void _initDescriptorSetLayout();
	void _deInitDescriptorSetLayout();

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

	void _initVertexBuffer();
	void _deInitVertexBuffer();

	void _initUniformBuffers();
	void _deInitUniformBuffer();

	void _initDescriptorPool();
	void _deInitDescriptorPool();

	void _initDescriptorSet();
	void _deInitDescriptorSet();

	void _initCommandBuffers();
	void _deInitCommandBuffers();

	void _initSynchronizations();
	void _deInitSynchronizations();

	VkShaderModule _createShaderModule(const std::vector<char>& code);

	void _setupDebug();
	void _initDebug();
	void _deInitDebug();

	UniformBufferObject					_ubo = {};
	float								_time = 0.0;

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
	VkDescriptorSetLayout				_descriptorSetLayout;
	std::vector<VkDescriptorSet>		_descriptorSets;
	VkDescriptorPool					_descriptorPool;
	VkPipeline							_graphicsPipeline;
	VkCommandPool						_commandPool;
	std::vector<VkCommandBuffer>		_commandBuffers;

	VkBuffer							_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory						_vertexBufferMemory;

	VkBuffer							_indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory						_indexBufferMemory;

	std::vector<VkBuffer>				_uniformBuffers;
	std::vector<VkDeviceMemory>			_uniformBuffersMemory;

	std::vector<VkSemaphore>		    _imageAvailableSemaphores;
	std::vector<VkSemaphore>			_renderFinishedSemaphores;
	size_t								_currentFrame = 0;

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

