
#include "Platform.h"
#include <vector>
#include "Context.h"
#include "Camera.hpp"
#include <memory.h>
#include "Renderer.h"

class Window;
class Camera;
struct Drawable;
struct Vertex;
#pragma once

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class ResourceManager;


struct FrameData 
{
    vk::Semaphore                   renderFinishedSemaphore{};
    std::vector<vk::Semaphore>      imageReadySemaphores{};
    std::vector<vk::CommandBuffer>  cmdBuffers;
};

class VulkanRenderer : public Renderer
{
public:
	VulkanRenderer(Window *window);
	~VulkanRenderer();

    bool Run();
    void DrawFrame();
    void Resize(int width, int height);
    Camera * GetCamera();
    void OnSceneChanged();

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

    void addRenderNodes(std::vector<std::shared_ptr<Drawable>> nodes);
private:
	void _beginRender();
	void _endRender(std::vector<VkSemaphore> waitSemaphores);
	void _updateUniformBuffer();

    void _cleanupSwapchain();

	void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
	
	void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

	void _initSwapchain();
	void _deInitSwapchain();

	void _initSwapchainImages();
	void _deInitSwapchainImages();

	void _initDepthStencilImage();
	void _deInitDepthStencilImage();

	void _initRenderPass();
	void _deInitRenderPass();

	void _initGraphicsPipeline();
	void _deInitGraphicsPipeline();

	void _initFramebuffers();
	void _deInitFramebuffers();

	void _initTextureImage();
	void _deInitTextureImage();

	void _initTextureImageView();
	void _deInitTextureImageView();

    void _initTextureImageSampler();
    void _deInitTextureImageSampler();

	void _initUniformBuffers();
	void _deInitUniformBuffers();

    void _initDescriptorSetLayout();
    void _deInitDescriptorSetLayout();

	void _initDescriptorPool();
	void _deInitDescriptorPool();

	void _initDescriptorSet();
	void _deInitDescriptorSet();

	void _initSynchronizations();
	void _deInitSynchronizations();

    void     _createCommandBuffers(std::vector<std::shared_ptr<Drawable>> &nodes);

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
	uint32_t							_graphicsQueueFamilyIndex = 0;

    VmaAllocator                        _memoryAllocator;

	ResourceManager                 *   _resourceManager;

	vk::SurfaceKHR					    _surface;
	std::vector<vk::Image>				_swapchainImages;
	std::vector<vk::ImageView>			_swapchainImageViews;
    std::vector<FrameData>              _framesData;
	std::vector<VkFramebuffer>			_framebuffers;
	VkPipelineLayout					_pipelineLayout;
	vk::DescriptorSetLayout				_descriptorSetLayout;
	std::vector<VkDescriptorSet>		_descriptorSets;
	vk::DescriptorPool					_descriptorPool;
	VkPipeline							_graphicsPipeline;
	std::vector<VkCommandBuffer>		_commandBuffers;

	vk::Image							_textureImage;
	VmaAllocation						_textureImageMemory;
    vk::ImageView                       _textureImageView;
    vk::Sampler                         _textureImageSampler;

	std::vector<VkBuffer>				_uniformBuffers;
	std::vector<VmaAllocation>			_uniformBuffersMemory;

	std::vector<vk::Semaphore>		    _imageAvailableSemaphores;

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

	Window                            * _window = nullptr;

    Camera                            * _camera;
};

