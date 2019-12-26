
#include "Platform.h"
#include <vector>
#include "Context.h"

const int MAX_FRAMES_IN_FLIGHT = 2;
class Window;

#pragma once

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class ResourceManager;

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

    void _cleanupSwapchain();

    void _copyBuffer(vk::Buffer srcBuffer,
        vk::Buffer dstBuffer, vk::DeviceSize size);

	void _initSwapchain();
	void _deInitSwapchain();

	void _initSwapchainImages();
	void _deInitSwapchainImages();

	void _initDepthStencilImage();
	void _deInitDepthStencilImage();

	void _initRenderPass();
	void _deInitRenderPass();

	void _initDescriptorSetLayout();
	void _deInitDescriptorSetLayout();

	void _initGraphicsPipeline();
	void _deInitGraphicPipeline();

	void _initFramebuffers();
	void _deInitFramebuffer();

	void _initTextureImage();
	void _deInitTextureImage();

	void _initVertexBuffer();
	void _deInitVertexBuffer();

    void _initIndexBuffer();
    void _deInitIndexBuffer();

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

	VulkanContext					*	_context;

	UniformBufferObject					_ubo = {};
	float								_time = 0.0;

	vk::Instance						_instance;
	vk::PhysicalDevice					_gpu;
	vk::Device							_device;
	vk::Queue							_queue;
    vk::CommandPool						_commandPool;
    vk::PhysicalDeviceProperties		_gpuProperties;
	vk::PhysicalDeviceMemoryProperties	_gpuMemoryProperties;
	uint32_t							_graphicFamilyIndex = 0;

    VmaAllocator                        _memoryAllocator;

	ResourceManager					 *  _resourceManager;

	vk::SurfaceKHR					    _surface;
	std::vector<vk::Image>				_swapchainImages;
	std::vector<vk::ImageView>			_swapchainImageViews;
	std::vector<VkFramebuffer>			_framebuffers;
	VkPipelineLayout					_pipelineLayout;
	VkDescriptorSetLayout				_descriptorSetLayout;
	std::vector<VkDescriptorSet>		_descriptorSets;
	VkDescriptorPool					_descriptorPool;
	VkPipeline							_graphicsPipeline;
	std::vector<VkCommandBuffer>		_commandBuffers;

	VkBuffer							_vertexBuffer = VK_NULL_HANDLE;
    VmaAllocation						_vertexBufferMemory;

	VkBuffer							_indexBuffer = VK_NULL_HANDLE;
    VmaAllocation						_indexBufferMemory;

	VkImage								_textureImage;
	VkDeviceMemory						_textureImageMemory;

	std::vector<VkBuffer>				_uniformBuffers;
	std::vector<VmaAllocation>			_uniformBuffersMemory;

	std::vector<VkSemaphore>		    _imageAvailableSemaphores;
	std::vector<VkSemaphore>			_renderFinishedSemaphores;
	size_t								_currentFrame = 0;

	vk::Extent2D						_swapchainExtent;

	VkImage								_depthStencilImage = VK_NULL_HANDLE;
    VmaAllocation						_depthStencilImageMemory = VK_NULL_HANDLE;
	VkImageView							_depthStencilImageView = VK_NULL_HANDLE;
	vk::Format							_depthStencilFormat;
	boolean								_stencilAvailable = false;

	VkRenderPass						_renderPass = VK_NULL_HANDLE;

	vk::SurfaceFormatKHR				_surfaceFormat;
	VkSwapchainKHR						_swapchain = VK_NULL_HANDLE;
	uint32_t							_swapchainImageCount;
	uint32_t							_activeSwapchainImageId = UINT32_MAX;

	VkFence								_swapchainImageAvailable;

	Window * _window = nullptr;
};

