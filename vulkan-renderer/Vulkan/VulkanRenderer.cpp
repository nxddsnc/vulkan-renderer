#define VMA_IMPLEMENTATION
#include "VulkanRenderer.h"
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include "BUILD_OPTIONS.h"
#include "Shared.h"
#include "Window.h"
#include <array>
#include "Scene.h"
#include <chrono>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <array>
#include "ResourceManager.h"
#include "PipelineManager.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Renderable.h"
#include "Context.h"
#include "Skybox.h"
#include "Axis.h"
#include "SHLight.h"
#include "Framebuffer.h"
#include "MyTexture.h"
#include "PostEffect/PostEffect.h"
#include "PostEffect/Bloom.h"
#include "PostEffect/ToneMapping.h"
#include <assert.h>
#include "RenderSceneForward.h"
#include "RenderSceneDeferred.h"
#include "MyScene.h"
#include "ShadowMap.h"
VulkanRenderer::VulkanRenderer(Window *window)
{
    _window = window;
    _swapchainExtent = {WIDTH, HEIGHT};
    _context = new VulkanContext(window);

    _pipelineManager = new PipelineManager(this);

    _gpu = _context->GetPhysicalDevice();
    _device = _context->GetLogicalDevice();
    _instance = _context->GetInstance();
    _queue = _context->GetDeviceQueue();
    _surface = _context->GetSuface();
    _commandPool = _context->GetCommandPool();
    _surface = _context->GetSuface();
    _surfaceFormat = _context->GetSurfaceFormat();
    _graphicsQueueFamilyIndex = _context->GetGraphicsQueueFamilyIndex();

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = VkPhysicalDevice(_gpu);
    allocatorInfo.device = VkDevice(_device);
    allocatorInfo.instance = VkInstance(_instance);
    vmaCreateAllocator(&allocatorInfo, &_memoryAllocator);

	_resourceManager = new ResourceManager(_device, _commandPool, _queue, _graphicsQueueFamilyIndex, _memoryAllocator, _gpu);
	_initDescriptorPool();
	_resourceManager->m_descriptorPool = _descriptorPool;

	//_renderScene = std::make_shared<RenderSceneForward>(_resourceManager, _pipelineManager, _swapchainExtent.width, _swapchainExtent.height);

	_renderScene = std::make_shared<RenderSceneDeferred>(_resourceManager, _pipelineManager, _swapchainExtent.width, _swapchainExtent.height);

	_renderScene->m_pSkybox = std::make_shared<Skybox>(_resourceManager, _pipelineManager, _context);
	_renderScene->m_pAxis = std::make_shared<Axis>(_resourceManager, _pipelineManager);

	_initSwapchain();
    _initSwapchainImages();
    _initDepthStencilImage();
    _initFramebuffers();
    _initSynchronizations();

	_initOffscreenRenderTargets();
}

VulkanRenderer::~VulkanRenderer()
{
    vkDeviceWaitIdle(_device);
    vkQueueWaitIdle(_queue);

	_deInitOffscreenRenderTargets();

	_renderScene = nullptr;

    delete _window;
	delete _pipelineManager;

    _deInitSynchronizations();
    _deInitFramebuffers();
    _deInitDepthStencilImage();
    _deInitSwapchainImages();
    _deInitSwapchain();

	_deInitDescriptorPool();

	delete _resourceManager;

    vmaDestroyAllocator(_memoryAllocator);

    delete _context;
}

bool VulkanRenderer::Run()
{
    if (nullptr != _window) 
    {
        return _window->Update();
    }
    return true;
}

void VulkanRenderer::Resize(int width, int height)
{
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(_window->GetGLFWWindow(), &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(_device);

    _swapchainExtent.width = width;
    _swapchainExtent.height = height;
    _window->Resize(width, height);
    _cleanupSwapchain();

    _initSwapchain();
    _initSwapchainImages();
    _initDepthStencilImage();
    _initFramebuffers();
    _initDescriptorPool();
}

vk::Instance VulkanRenderer::GetVulkanInstance()
{
    return _instance;
}

vk::PhysicalDevice VulkanRenderer::GetPhysicalDevice()
{
    return _gpu;
}

vk::Device VulkanRenderer::GetVulkanDevice()
{
    return _device;
}

vk::Queue VulkanRenderer::GetVulkanDeviceQueue()
{
    return _queue;
}

vk::PhysicalDeviceProperties VulkanRenderer::GetPhycicalDeviceProperties()
{
    return _gpuProperties;
}

vk::PhysicalDeviceMemoryProperties VulkanRenderer::GetPhysicalDeviceMemoryProperties()
{
    return _gpuMemoryProperties;
}

uint32_t VulkanRenderer::GetGraphicFamilyIndex()
{
    return _graphicsQueueFamilyIndex;
}

vk::RenderPass VulkanRenderer::GetVulkanRenderPass()
{
    return _renderPass->Get();
}

vk::Framebuffer VulkanRenderer::GetActiveFramebuffer()
{
    return _framesData[_activeSwapchainImageId].framebuffer->m_vkFramebuffer;
}

uint32_t VulkanRenderer::GetSwapchainImageCount()
{
    return _swapchainImageCount;
}

vk::SurfaceFormatKHR VulkanRenderer::GetSurfaceFormat()
{
    return _surfaceFormat;
}

vk::Format VulkanRenderer::GetDepthFormat()
{
    return _depthStencilImage->format;
}

std::shared_ptr<MyCamera> VulkanRenderer::GetCamera()
{
    return _renderScene->m_pCamera;
}

void VulkanRenderer::OnSceneChanged()
{
}

void VulkanRenderer::GetExtendSize(uint32_t &width, uint32_t &height)
{
    width = _swapchainExtent.width;
    height = _swapchainExtent.height;
}

void VulkanRenderer::LoadSkybox(const char * path)
{
	_renderScene->m_pSkybox->LoadFromDDS(path, _device, _descriptorPool);
	_createCommandBuffers();
}

void VulkanRenderer::DrawFrame()
{
    _beginRender();
    

    vk::PipelineStageFlags pipelineStageFlag = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo( uint32_t(1),
                               &(_imageAvailableSemaphores[_currentFrame]),
                               &pipelineStageFlag,
                               (uint32_t)_framesData[_activeSwapchainImageId].cmdBuffers.size(),
                               _framesData[_activeSwapchainImageId].cmdBuffers.data(),
                               uint32_t(1),
                               &(_framesData[_currentFrame].renderFinishedSemaphore));

    _queue.submit(1, &submitInfo, {});

    _endRender({ _framesData[_currentFrame].renderFinishedSemaphore });
}

void VulkanRenderer::_beginRender()
{
	vkQueueWaitIdle(_queue);
	vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &_activeSwapchainImageId);
    _updateUniformBuffer();
    //vkWaitForFences(_device, 1, &_swapchainImageAvailable, VK_TRUE, UINT64_MAX);
    //vkResetFences(_device, 1, &_swapchainImageAvailable);
    //vkQueueWaitIdle(_queue);
}

void VulkanRenderer::_endRender(std::vector<VkSemaphore> waitSemaphores)
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
    vkQueuePresentKHR(_queue, &presentInfo);
    _currentFrame = (_currentFrame + 1) % _swapchainImageCount;
}

void VulkanRenderer::_updateUniformBuffer()
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    //UniformBufferObject ubo = {};
    //ubo.model = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    //ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //ubo.proj = glm::perspective(glm::radians(45.0f), _swapchainExtent.width / (float)_swapchainExtent.height, 0.1f, 10.0f);
    //ubo.proj[1][1] *= -1;

	_renderScene->UpdateUniforms();
}

void VulkanRenderer::_cleanupSwapchain()
{
    _deInitSwapchainImages();
    _deInitDepthStencilImage();
    _deInitFramebuffers();
    _deInitDescriptorPool();
    _deInitSwapchain();
}

void VulkanRenderer::_initSwapchain()
{
    vk::SurfaceCapabilitiesKHR surfaceCapibilities = _gpu.getSurfaceCapabilitiesKHR(_surface);
    if (_swapchainImageCount > surfaceCapibilities.maxImageCount)
    {
        _swapchainImageCount = surfaceCapibilities.maxImageCount;
    }
    if (_swapchainImageCount < surfaceCapibilities.minImageCount)
    {
        _swapchainImageCount = surfaceCapibilities.minImageCount;
    }

    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    {
        std::vector<vk::PresentModeKHR> presentModes = _gpu.getSurfacePresentModesKHR(_surface);
        for (auto m : presentModes)
        {
            if (m == vk::PresentModeKHR::eMailbox)
            {
                presentMode = m;
                break;
            }
        }
    }

    vk::SwapchainCreateInfoKHR createInfo({
        {},
        _surface,
        _swapchainImageCount,
        _surfaceFormat.format,
        _surfaceFormat.colorSpace,
        _window->GetWindowExtent(),
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        presentMode,
        true,
        nullptr
    });
    _swapchain = _device.createSwapchainKHR(createInfo);

    _framesData.resize(_swapchainImageCount);
}

void VulkanRenderer::_deInitSwapchain()
{
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

void VulkanRenderer::_initSwapchainImages()
{
	std::vector<vk::Image> images = _device.getSwapchainImagesKHR(_swapchain);

	_swapchainImages.resize(_swapchainImageCount);

	for (int i = 0; i < images.size(); ++i)
	{
		_swapchainImages[i] = std::make_shared<VulkanTexture>();
		_swapchainImages[i]->image = images[i];
		vk::ImageViewCreateInfo createInfo({},
			_swapchainImages[i]->image,
			vk::ImageViewType::e2D,
			_surfaceFormat.format,
			vk::ComponentMapping({
				vk::ComponentSwizzle::eR,
				vk::ComponentSwizzle::eG,
				vk::ComponentSwizzle::eB,
				vk::ComponentSwizzle::eA
			}),
			vk::ImageSubresourceRange({
				vk::ImageAspectFlagBits::eColor,
				0,
				1,
				0,
				1
			}));
		_swapchainImages[i]->imageView = _device.createImageView(createInfo);
	}
}

void VulkanRenderer::_deInitSwapchainImages()
{
    for (uint32_t i = 0; i < _swapchainImageCount; ++i)
    {
        _device.destroyImageView(_swapchainImages[i]->imageView);
    }
}

void VulkanRenderer::_initDepthStencilImage()
{
	vk::Format format;
    {
        std::vector<vk::Format> tryFormats{
            vk::Format::eD24UnormS8Uint,
            vk::Format::eD16UnormS8Uint,
            vk::Format::eD32SfloatS8Uint,
            vk::Format::eS8Uint,
            vk::Format::eD32Sfloat,
            vk::Format::eD16Unorm
        };
        for (auto f : tryFormats)
        {
            vk::FormatProperties formatProperties = _gpu.getFormatProperties(f);
            if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
            {
				format = f;
                break;
            }
        }
        if (format == vk::Format::eUndefined)
        {
            assert(1 && "wrong format found!");
            exit(-1);
        }
        if (format == vk::Format::eD24UnormS8Uint ||
			format == vk::Format::eD16UnormS8Uint ||
			format == vk::Format::eD32SfloatS8Uint ||
			format == vk::Format::eS8Uint)
        {
            _stencilAvailable = true;
        }
    }


	MyImageFormat myFormat;
	switch (format)
	{
	case vk::Format::eD24UnormS8Uint:
		myFormat = MY_IMAGEFORMAT_D24S8_UINT;
		break;
	default:
		assert(true);
		break;
	}

	char name[32] = "swapchain-depth";
	std::shared_ptr<MyTexture> depthTexture = std::make_shared<MyTexture>();
	depthTexture->m_pImage = std::make_shared<MyImage>(name, _swapchainExtent.width, _swapchainExtent.height, myFormat, true);

	_depthStencilImage = _resourceManager->CreateCombinedTexture(depthTexture);
}

void VulkanRenderer::_deInitDepthStencilImage()
{
	_depthStencilImage = nullptr;
}

void VulkanRenderer::_initFramebuffers() 
{
    _renderPass = std::make_shared<RenderPass>(&_device);
    vk::AttachmentDescription colorAttachment({
        {},
        _surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    });

    _renderPass->AddAttachment(colorAttachment);

	vk::AttachmentDescription depthAttachment({
		{},
		_depthStencilImage->format,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eStore,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	});
    _renderPass->AddAttachment(depthAttachment);

    for (int i = 0; i < _swapchainImageCount; ++i)
    {
		char name[64];
		sprintf(name, "swapchain-%d", i);
		_framesData[i].framebuffer = std::make_shared<Framebuffer>(name, _resourceManager, _renderPass, _swapchainImages[i], _depthStencilImage, _swapchainExtent.width, _swapchainExtent.height);
    }
}

void VulkanRenderer::_deInitFramebuffers()
{
    // TODO: create and destroy should be in the same class.
	_renderPass = nullptr;
    for (uint32_t i = 0; i < _swapchainImageCount; ++i)
    {
		_framesData[i].framebuffer = nullptr;
    }
}

void VulkanRenderer::_initOffscreenRenderTargets()
{
	std::shared_ptr<Bloom> bloom = std::make_shared<Bloom>(_resourceManager, _pipelineManager, _swapchainExtent.width, _swapchainExtent.height);
	m_postEffects.push_back(bloom);

    std::shared_ptr<ToneMapping> toneMapping = std::make_shared<ToneMapping>(_resourceManager, _pipelineManager, _swapchainExtent.width, _swapchainExtent.height);
    m_postEffects.push_back(toneMapping);
}

void VulkanRenderer::_deInitOffscreenRenderTargets()
{
	m_postEffects.clear();
}

void VulkanRenderer::_initDescriptorPool()
{
    // TODO: Assign descriptor and max set according to pipeline information.
    std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].descriptorCount = 10;
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[1].descriptorCount = 10;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;

    vk::DescriptorPoolCreateInfo poolInfo({{},
                                           static_cast<uint32_t>(1024),
                                           static_cast<uint32_t>(poolSizes.size()),
                                           poolSizes.data()});
    _descriptorPool = _device.createDescriptorPool(poolInfo);
}

void VulkanRenderer::_deInitDescriptorPool()
{
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
}

void VulkanRenderer::_initSynchronizations()
{
    VkFenceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(_device, &createInfo, nullptr, &_swapchainImageAvailable);

    vk::SemaphoreCreateInfo semaphoreInfo({});

    _imageAvailableSemaphores.resize(_swapchainImageCount);

    for (size_t i = 0; i < _swapchainImageCount; i++)
    {
        _imageAvailableSemaphores[i] = _device.createSemaphore(semaphoreInfo);
        _framesData[i].renderFinishedSemaphore = _device.createSemaphore(semaphoreInfo);
    }
}

void VulkanRenderer::_deInitSynchronizations()
{
    vkDestroyFence(_device, _swapchainImageAvailable, nullptr);

    for (size_t i = 0; i < _swapchainImageCount; i++)
    {
        _device.destroySemaphore(_imageAvailableSemaphores[i]);
        _device.destroySemaphore(_framesData[i].renderFinishedSemaphore);
    }
}

void VulkanRenderer::_createCommandBuffers()
{
    for (uint32_t i = 0; i < _swapchainImageCount; ++i)
    {
		_framesData[i].cmdBuffers.clear();

		vk::CommandBufferAllocateInfo commandBufferAllocateInfo({
		   _commandPool,
		   vk::CommandBufferLevel::ePrimary,
		   static_cast<uint32_t>(1)
		});
		vk::CommandBuffer commandBuffer;
		_device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);

		vk::CommandBufferBeginInfo beginInfo({
			vk::CommandBufferUsageFlagBits::eSimultaneousUse,
			nullptr
		});

		commandBuffer.begin(beginInfo);

		_renderScene->Draw(commandBuffer);

		std::shared_ptr<Framebuffer> offscreenFramebuffer = _renderScene->GetFramebuffer();
		std::shared_ptr<Framebuffer> inputFrameubffer = offscreenFramebuffer;
		for (int index = 0; index < m_postEffects.size() - 1; ++index)
		{
			m_postEffects[index]->Draw(commandBuffer, { inputFrameubffer });
			inputFrameubffer = m_postEffects[index]->GetFramebuffer();
		}

		// Tonemapping 
		// FIXME: Not a very solid method of post processing.
		////std::vector<std::shared_ptr<Framebuffer>> framebuffers = { inputFrameubffer, _renderScene->m_pShadowMap->m_pFramebuffer };
		std::vector<std::shared_ptr<Framebuffer>> framebuffers = { inputFrameubffer, offscreenFramebuffer };
		m_postEffects.back()->Draw(commandBuffer, framebuffers, _framesData[i].framebuffer);

		commandBuffer.end();

		_framesData[i].cmdBuffers.push_back(commandBuffer);
	}
}

void VulkanRenderer::AddScene(std::shared_ptr<MyScene> scene)
{
	_renderScene->AddScene(scene);

    _createCommandBuffers();
}