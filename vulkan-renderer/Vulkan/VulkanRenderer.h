#include <vector>
#include <memory.h>
#include <unordered_map>
#include "Pipeline.h"
#include "Platform.h"

class Window;
class VulkanCamera;
class Drawable;
struct Vertex;
class SHLight;
class Axis;
class PostEffect;
class Bloom;
#pragma once

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class ResourceManager;
class PipelineManager;
class VulkanContext;
class VulkanCamera;
class Skybox;
class Framebuffer;
class RenderPass;
struct VulkanTexture;
class RenderScene;

struct FrameData 
{
    vk::Semaphore                   renderFinishedSemaphore{};
    std::vector<vk::Semaphore>      imageReadySemaphores{};
    std::vector<vk::CommandBuffer>  cmdBuffers;
    std::shared_ptr<Framebuffer>    framebuffer;
};

class VulkanRenderer
{
public:
    VulkanRenderer(Window *window);
    ~VulkanRenderer();

    bool Run();
    void DrawFrame();
    void Resize(int width, int height);
	std::shared_ptr<VulkanCamera> GetCamera();
    void OnSceneChanged();
    void GetExtendSize(uint32_t &width, uint32_t &height);

	void LoadSkybox(const char* path);

    vk::Instance GetVulkanInstance();
    vk::PhysicalDevice GetPhysicalDevice();
    vk::Device GetVulkanDevice();
    vk::Queue GetVulkanDeviceQueue();
    vk::PhysicalDeviceProperties GetPhycicalDeviceProperties();
    vk::PhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties();
    uint32_t GetGraphicFamilyIndex();
    vk::RenderPass GetVulkanRenderPass();
    vk::Framebuffer GetActiveFramebuffer();
    uint32_t GetSwapchainImageCount();
    vk::SurfaceFormatKHR GetSurfaceFormat();
    vk::Format GetDepthFormat();
    vk::RenderPass GetOffscreenRenderPass();

    void AddRenderNodes(std::vector<std::shared_ptr<Drawable>> nodes);
private:

    void _beginRender();
    void _endRender(std::vector<VkSemaphore> waitSemaphores);
    void _updateUniformBuffer();

    void _cleanupSwapchain();

    void _initSwapchain();
    void _deInitSwapchain();

    void _initSwapchainImages();
    void _deInitSwapchainImages();

    void _initDepthStencilImage();
    void _deInitDepthStencilImage();

    void _initFramebuffers();
    void _deInitFramebuffers();

	void _initOffscreenRenderTargets();
	void _deInitOffscreenRenderTargets();

    void _initDescriptorPool();
    void _deInitDescriptorPool();

    void _initSynchronizations();
    void _deInitSynchronizations();

    //void _createCommandBuffers(std::vector<std::shared_ptr<Drawable>> drawables);
    void _createCommandBuffers();
	VulkanContext                        *        _context;

	UniformBufferObject                           _ubo = {};
	float                                         _time = 0.0;

	vk::Instance                                  _instance;
	vk::PhysicalDevice                            _gpu;
	vk::Device                                    _device;
	vk::Queue                                     _queue;
	vk::CommandPool                               _commandPool;
	vk::PhysicalDeviceProperties                  _gpuProperties;
	vk::PhysicalDeviceMemoryProperties            _gpuMemoryProperties;
	uint32_t                                      _graphicsQueueFamilyIndex = 0;

	VmaAllocator                                  _memoryAllocator;

	ResourceManager                     *         _resourceManager;

	PipelineManager                     *         _pipelineManager;

	vk::SurfaceKHR                                _surface;
	/*std::vector<vk::Image>                        _swapchainImages;
	std::vector<vk::ImageView>                    _swapchainImageViews;*/
	std::vector<std::shared_ptr<VulkanTexture>>   _swapchainImages;
    std::vector<FrameData>                        _framesData;
	std::shared_ptr<RenderPass>				      _renderPass;
											      
	std::shared_ptr<Framebuffer>			      _offscreenFramebuffer;
											      
    std::vector<VkDescriptorSet>                  _descriptorSets;
    vk::DescriptorSetLayout                       _frameDescriptorSetLayout;
    vk::DescriptorPool                            _descriptorPool;
    std::vector<VkCommandBuffer>                  _commandBuffers;

    std::unordered_map<PipelineId, std::vector<std::shared_ptr<Drawable>>> _drawableMap;

	std::shared_ptr<RenderScene>				  _renderScene;

    std::vector<vk::Semaphore>                    _imageAvailableSemaphores;
											      
    size_t                                        _currentFrame = 0;
											      
    vk::Extent2D                                  _swapchainExtent;

	std::shared_ptr<VulkanTexture>				  _depthStencilImage;

    boolean                                       _stencilAvailable = false;
											      
    vk::SurfaceFormatKHR                          _surfaceFormat;
    VkSwapchainKHR                                _swapchain = VK_NULL_HANDLE;
    uint32_t                                      _swapchainImageCount;
    uint32_t                                      _activeSwapchainImageId = UINT32_MAX;
											      
    VkFence                                       _swapchainImageAvailable;
											      
    Window                            *           _window = nullptr;
											      
	std::vector<std::shared_ptr<PostEffect>>      m_postEffects;
};

