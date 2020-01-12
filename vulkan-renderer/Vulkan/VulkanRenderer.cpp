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

VulkanRenderer::VulkanRenderer(Window *window)
{
	_window = window;
	_swapchainExtent = {WIDTH, HEIGHT};
	_context = new VulkanContext(window);
	_camera = new Camera();
	_camera->type = Camera::CameraType::lookat;
	_camera->setPosition(glm::vec3(0, 0, 0));
	_camera->setRotation(glm::vec3(-45, 0, 45));
	_camera->setPerspective(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 10.0f);

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

	_resourceManager = new ResourceManager(_device, _commandPool, _queue, _graphicsQueueFamilyIndex, _memoryAllocator);

	_initSwapchain();
	_initSwapchainImages();
	_initDepthStencilImage();
	_initFramebuffers();
	_initUniformBuffers();
	_initDescriptorPool();
	_initDescriptorSet();
	_initSynchronizations();
}

VulkanRenderer::~VulkanRenderer()
{
	vkDeviceWaitIdle(_device);
	vkQueueWaitIdle(_queue);
	delete _window;
	_deInitSynchronizations();
	_deInitDescriptorSet();
	_deInitDescriptorPool();
	_deInitUniformBuffers();
	_deInitFramebuffers();
	_deInitDepthStencilImage();
	_deInitSwapchainImages();
	_deInitSwapchain();

	delete _resourceManager;
	vmaDestroyAllocator(_memoryAllocator);

	delete _camera;
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
	while(width == 0 || height == 0)
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
	_initUniformBuffers();
	_initDepthStencilImage();
	_initRenderPass();
    _initGraphicsPipeline();
    _initFramebuffers();
	_initDescriptorPool();
	_initDescriptorSet(); 
}

VkInstance VulkanRenderer::GetVulkanInstance()
{
	return _instance;
}

VkPhysicalDevice VulkanRenderer::GetPhysicalDevice()
{
	return _gpu;
}

VkDevice VulkanRenderer::GetVulkanDevice()
{
	return _device;
}

VkQueue VulkanRenderer::GetVulkanDeviceQueue()
{
	return _queue;
}

VkPhysicalDeviceProperties VulkanRenderer::GetPhycicalDeviceProperties()
{
	return _gpuProperties;
}

VkPhysicalDeviceMemoryProperties VulkanRenderer::GetPhysicalDeviceMemoryProperties()
{
	return _gpuMemoryProperties;
}

uint32_t VulkanRenderer::GetGraphicFamilyIndex()
{
	return _graphicsQueueFamilyIndex;
}

VkRenderPass VulkanRenderer::GetVulkanRenderPass()
{
	return _renderPass;
}

VkFramebuffer VulkanRenderer::GetActiveFramebuffer()
{
	return _framebuffers[_activeSwapchainImageId];
}

uint32_t VulkanRenderer::GetSwapchainImageCount()
{
	return uint32_t();
}

vk::SurfaceFormatKHR VulkanRenderer::GetSurfaceFormat()
{
	return _surfaceFormat;
}

vk::Format VulkanRenderer::GetDepthFormat()
{
	return _depthStencilFormat;
}

Camera * VulkanRenderer::GetCamera()
{
    return _camera;
}

void VulkanRenderer::OnSceneChanged()
{
}

void VulkanRenderer::GetExtendSize(uint32_t &width, uint32_t &height)
{
	width = _swapchainExtent.width;
	height = _swapchainExtent.height;
}

void VulkanRenderer::DrawFrame()
{
	_beginRender();
	
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = _framesData[_activeSwapchainImageId].cmdBuffers.size();
	submitInfo.pCommandBuffers = _framesData[_activeSwapchainImageId].cmdBuffers.data();

	std::vector<VkSemaphore> signalSemaphores{ _framesData[_currentFrame].renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);

	_endRender(signalSemaphores);
}

void VulkanRenderer::_beginRender()
{
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
    vkQueueWaitIdle(_queue);
	_currentFrame = (_currentFrame + 1) % _swapchainImageCount;
}

void VulkanRenderer::_updateUniformBuffer()
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo = {};
	//ubo.model = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//ubo.proj = glm::perspective(glm::radians(45.0f), _swapchainExtent.width / (float)_swapchainExtent.height, 0.1f, 10.0f);
	//ubo.proj[1][1] *= -1;

    ubo.model = glm::mat4(1.0f);
    ubo.view = _camera->matrices.view;
    ubo.proj = _camera->matrices.perspective;
    ubo.proj[1][1] *= -1;

	void* data;
	vmaMapMemory(_memoryAllocator, _uniformBuffersMemory[_activeSwapchainImageId], &data);
	memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(_memoryAllocator, _uniformBuffersMemory[_activeSwapchainImageId]);
}

void VulkanRenderer::_cleanupSwapchain()
{
    _deInitSwapchainImages();
    _deInitUniformBuffers();
    _deInitDepthStencilImage();
    _deInitRenderPass();
    _deInitGraphicsPipeline();
    _deInitFramebuffers();
    _deInitDescriptorPool();
    _deInitDescriptorSet();
    _deInitSwapchain();
}

VkShaderModule VulkanRenderer::_createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkShaderModule shaderModule;
	vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule);
	return shaderModule;
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
	_swapchainImageViews.resize(_swapchainImageCount);
	_swapchainImages = _device.getSwapchainImagesKHR(_swapchain);

	for (uint32_t i = 0; i < _swapchainImages.size(); ++i)
	{
		vk::ImageViewCreateInfo createInfo({
			{},
			_swapchainImages[i],
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
			})
		});
		_swapchainImageViews[i] = _device.createImageView(createInfo);
	}
}
void VulkanRenderer::_deInitSwapchainImages()
{
	for (uint32_t i = 0; i < _swapchainImageCount; ++i)
	{
		_device.destroyImageView(_swapchainImageViews[i]);
	}
}

void VulkanRenderer::_initDepthStencilImage()
{
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
				_depthStencilFormat = f;
				break;
			}
		}
		if (_depthStencilFormat == vk::Format::eUndefined)
		{
			assert(1 && "wrong format found!");
			exit(-1);
		}
		if (_depthStencilFormat == vk::Format::eD24UnormS8Uint ||
			_depthStencilFormat == vk::Format::eD16UnormS8Uint ||
			_depthStencilFormat == vk::Format::eD32SfloatS8Uint ||
			_depthStencilFormat == vk::Format::eS8Uint)
		{
			_stencilAvailable = true;
		}
	}

    uint32_t graphicsQueueFamilyIndex = _context->GetGraphicsQueueFamilyIndex();
    vk::ImageCreateInfo createInfo({
        {},
        vk::ImageType::e2D,
        _depthStencilFormat,
        vk::Extent3D({
            _swapchainExtent.width, 
            _swapchainExtent.height,
            1
        }),
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::SharingMode::eExclusive,
        1,
        &graphicsQueueFamilyIndex,
        vk::ImageLayout::eUndefined
    });
    _depthStencilImage = _device.createImage(createInfo);

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.pool = VK_NULL_HANDLE;
    vmaAllocateMemoryForImage(_memoryAllocator, _depthStencilImage, &allocInfo, &_depthStencilImageMemory, nullptr);
    vmaBindImageMemory(_memoryAllocator, _depthStencilImageMemory, _depthStencilImage);

    vk::ImageViewCreateInfo imageViewCreateInfo({
        {},
        _depthStencilImage,
        vk::ImageViewType::e2D,
        _depthStencilFormat,
        vk::ComponentMapping({
            vk::ComponentSwizzle::eR,
            vk::ComponentSwizzle::eG,
            vk::ComponentSwizzle::eB,
            vk::ComponentSwizzle::eA
        }),
        vk::ImageSubresourceRange({
            vk::ImageAspectFlagBits::eDepth | (_stencilAvailable ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits(0)),
            0,
            1,
            0,
            1
        })
    });
    _depthStencilImageView = _device.createImageView(imageViewCreateInfo);
}

void VulkanRenderer::_deInitDepthStencilImage()
{
    _device.destroyImageView(_depthStencilImageView);
    vmaDestroyImage(_memoryAllocator, _depthStencilImage, _depthStencilImageMemory);
}

void VulkanRenderer::_initFramebuffers()
{
	_framebuffers.resize(_swapchainImageCount);
	for (uint32_t i = 0; i < _swapchainImageCount; ++i)
	{
		std::array<vk::ImageView, 2> attachments{};
        attachments[0] = _swapchainImageViews[i];
		attachments[1] = _depthStencilImageView;

        vk::FramebufferCreateInfo createInfo({
            {},
            _renderPass,
            (uint32_t)attachments.size(),
            attachments.data(),
            _swapchainExtent.width,
            _swapchainExtent.height,
            uint32_t(1)
        });
        _framebuffers[i] = _device.createFramebuffer(createInfo);
	}
}

void VulkanRenderer::_deInitFramebuffers()
{
	for (uint32_t i = 0; i < _swapchainImageCount; ++i)
	{
		vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
	}
}

void VulkanRenderer::_initUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	_uniformBuffers.resize(_swapchainImages.size());
	_uniformBuffersMemory.resize(_swapchainImages.size());

	for (size_t i = 0; i < _swapchainImages.size(); i++) {
        VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        createInfo.size = bufferSize;
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        vmaCreateBuffer(_memoryAllocator, &createInfo, &allocInfo, &_uniformBuffers[i], &_uniformBuffersMemory[i], nullptr);
	}
}

void VulkanRenderer::_deInitUniformBuffers()
{
	for (size_t i = 0; i < _swapchainImages.size(); i++)
	{
        vmaDestroyBuffer(_memoryAllocator, _uniformBuffers[i], _uniformBuffersMemory[i]);
	}
}

void VulkanRenderer::_initDescriptorPool()
{
	std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].descriptorCount = static_cast<uint32_t>(_swapchainImages.size());
	poolSizes[1].descriptorCount = static_cast<uint32_t>(_swapchainImages.size());

	vk::DescriptorPoolCreateInfo poolInfo({{},
																				 static_cast<uint32_t>(_swapchainImages.size()),
																				 static_cast<uint32_t>(poolSizes.size()),
																				 poolSizes.data()});
	_descriptorPool = _device.createDescriptorPool(poolInfo);
}

void VulkanRenderer::_deInitDescriptorPool()
{
	vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
}

void VulkanRenderer::_initDescriptorSet()
{
	std::vector<VkDescriptorSetLayout> layouts(_swapchainImages.size(), _descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(_swapchainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	_descriptorSets.resize(_swapchainImages.size());
	vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data());

	for (size_t i = 0; i < _swapchainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = _uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = _textureImageView;
		imageInfo.sampler = _textureImageSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = _descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;				// Optional
		descriptorWrites[0].pTexelBufferView = nullptr; // Optional

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = _descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}
}

void VulkanRenderer::_deInitDescriptorSet()
{
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

void VulkanRenderer::_createCommandBuffers(std::vector<std::shared_ptr<Drawable>>& nodes)
{
    //_commandBuffers.resize(_swapchainImageCount);
    //for (uint32_t i = 0; i < _commandBuffers.size(); ++i)
    //{
    //    VkCommandBuffer commandBuffer = {};
    //    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    //    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    //    commandBufferAllocateInfo.commandPool = _commandPool;
    //    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    //    commandBufferAllocateInfo.commandBufferCount = 1;
    //    vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, &commandBuffer);
    //    _commandBuffers[i] = commandBuffer;
    //}

    //for (size_t i = 0; i < _commandBuffers.size(); ++i) {
    //    VkCommandBufferBeginInfo beginInfo = {};
    //    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
    //    beginInfo.pInheritanceInfo = nullptr; // Optional

    //    vkBeginCommandBuffer(_commandBuffers[i], &beginInfo);

    //    VkRenderPassBeginInfo renderPassInfo = {};
    //    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //    renderPassInfo.renderPass = _renderPass;
    //    renderPassInfo.framebuffer = _framebuffers[i];

    //    renderPassInfo.renderArea.offset = { 0, 0 };
    //    renderPassInfo.renderArea.extent = _swapchainExtent;

    //    std::array<VkClearValue, 2> clearValues{};
    //    clearValues[0].color.float32[0] = 0.0;
    //    clearValues[0].color.float32[1] = 0.0;
    //    clearValues[0].color.float32[2] = 0.0;
    //    clearValues[0].color.float32[3] = 1.0f;

    //    clearValues[1].depthStencil.depth = 1.0f;
    //    clearValues[1].depthStencil.stencil = 0;

    //    renderPassInfo.clearValueCount = clearValues.size();
    //    renderPassInfo.pClearValues = clearValues.data();

    //    vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    //    vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

    //    for (auto node : nodes)
    //    {
    //        VkBuffer vertexBuffers[] = { node->vertexBuffer };
    //        VkDeviceSize offsets[] = { 0 };
    //        vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);

    //        switch (node->mesh->m_indexType)
    //        {
    //        case 1:
    //            vkCmdBindIndexBuffer(_commandBuffers[i], node->indexBuffer, 0, VK_INDEX_TYPE_UINT8_EXT);
    //            break;
    //        case 2:
    //            vkCmdBindIndexBuffer(_commandBuffers[i], node->indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    //            break;
    //        case 4:
    //            vkCmdBindIndexBuffer(_commandBuffers[i], node->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    //            break;
    //        }

    //        vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[i], 0, nullptr);

    //        vkCmdDrawIndexed(_commandBuffers[i], node->mesh->m_indexNum, 1, 0, 0, 0);
    //    }

    //    vkCmdEndRenderPass(_commandBuffers[i]);
    //    vkEndCommandBuffer(_commandBuffers[i]);
    //}
}

void VulkanRenderer::AddRenderNodes(std::vector<std::shared_ptr<Drawable>> nodes)
{
   /* for (auto node : nodes)
    {
        _resourceManager->createNodeResource(node);
    }

    _createCommandBuffers(nodes);*/
}


// void VulkanRenderer::_initTextureImage()
// {
// 	int texWidth, texHeight, texChannels;
// 	stbi_uc* pixels = stbi_load("Textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
// 	VkDeviceSize imageSize = texWidth * texHeight * 4;

// 	if (!pixels) {
// 		throw std::runtime_error("failed to load texture image!");
// 	}

// 	VkBuffer stagingBuffer;
// 	VmaAllocation stagingBufferMemory;

//     VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
//     stagingBufferInfo.size = imageSize;
//     stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//     VmaAllocationCreateInfo stagingBufferAllocInfo = {};
//     stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
//     vmaCreateBuffer(_memoryAllocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferMemory, nullptr);

// 	void* data;
//     vmaMapMemory(_memoryAllocator, stagingBufferMemory, &data);
// 	memcpy(data, pixels, static_cast<size_t>(imageSize));
//     vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

// 	stbi_image_free(pixels);

//     uint32_t graphicsQueueFamilyIndex = _context->GetGraphicsQueueFamilyIndex();
//     vk::ImageCreateInfo createInfo({
//         {},
//         vk::ImageType::e2D,
//         vk::Format::eR8G8B8A8Unorm,
//         vk::Extent3D({
//             static_cast<uint32_t>(texWidth),
//             static_cast<uint32_t>(texHeight),
//             1
//         }),
//         1,
//         1,
//         vk::SampleCountFlagBits::e1,
//         vk::ImageTiling::eOptimal,
//         vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
//         vk::SharingMode::eExclusive,
//         1,
//         &graphicsQueueFamilyIndex,
//         vk::ImageLayout::eUndefined
//     });
//     VmaAllocationCreateInfo allocationCreateInfo = {};
//     allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
//     VkImage image;
//     vmaCreateImage(_memoryAllocator, &(VkImageCreateInfo(createInfo)), &allocationCreateInfo, &image, &_textureImageMemory, nullptr);
//     _textureImage = image;

// 	transitionImageLayout(_textureImage, vk::Format::eR8G8B8A8Unorm,
// 		vk::ImageLayout::eUndefined , vk::ImageLayout::eTransferDstOptimal);
// 	copyBufferToImage(stagingBuffer, _textureImage, 
// 		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
// 	transitionImageLayout(_textureImage, vk::Format::eR8G8B8A8Unorm,
// 		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

// 	vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);
// }

// void VulkanRenderer::_deInitTextureImage()
// {
// 	vmaDestroyImage(_memoryAllocator, _textureImage, _textureImageMemory);
// }

// void VulkanRenderer::_initTextureImageView()
// {
// 	vk::ImageViewCreateInfo createInfo({
// 		{},
// 		_textureImage,
// 		vk::ImageViewType::e2D,
// 		vk::Format::eR8G8B8A8Unorm,
// 		vk::ComponentMapping({
//             vk::ComponentSwizzle::eR,
// 		    vk::ComponentSwizzle::eG,
// 		    vk::ComponentSwizzle::eB,
// 		    vk::ComponentSwizzle::eA
// 	    }),
// 		vk::ImageSubresourceRange({
// 	        vk::ImageAspectFlagBits::eColor,
// 		    0,
// 		    1,
// 		    0,
// 		    1
//     	})
// 	});
//     _textureImageView = _device.createImageView(createInfo);
// }

// void VulkanRenderer::_deInitTextureImageView()
// {
//     _device.destroyImageView(_textureImageView);
// }

// void VulkanRenderer::_initTextureImageSampler()
// {
//     vk::SamplerCreateInfo createInfo({
//         {},
//         vk::Filter::eLinear,
//         vk::Filter::eLinear,
//         vk::SamplerMipmapMode::eLinear,
//         vk::SamplerAddressMode::eRepeat,
//         vk::SamplerAddressMode::eRepeat,
//         vk::SamplerAddressMode::eRepeat,
//         0.0,
//         vk::Bool32(true),
//         16.0,
//         vk::Bool32(false),
//         vk::CompareOp::eAlways,
//         0.0,
//         0.0,
//         vk::BorderColor::eFloatOpaqueWhite,
//         vk::Bool32(false)
//     });

//     _textureImageSampler = _device.createSampler(createInfo);
// }

// void VulkanRenderer::_deInitTextureImageSampler()
// {
//     _device.destroySampler(_textureImageSampler);
// }

// void VulkanRenderer::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
// {
// 	/*vk::CommandBuffer commandBuffer = _beginSingleTimeCommand();

// 	vk::ImageMemoryBarrier barrier({
// 		{},
// 		{},
// 		oldLayout,
// 		newLayout,
// 		{},
// 		{},
// 		_textureImage,
// 		vk::ImageSubresourceRange({
// 		vk::ImageAspectFlagBits::eColor,
// 		(uint32_t)0,
// 		(uint32_t)1,
// 		(uint32_t)0,
// 		(uint32_t)1,
// 	})
// 	});

// 	std::array<vk::ImageMemoryBarrier, 1> barriers = { barrier };


// 	vk::PipelineStageFlagBits sourceStage;
// 	vk::PipelineStageFlagBits destinationStage;

// 	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
// 		barrier.srcAccessMask = {};
// 		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

// 		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
// 		destinationStage = vk::PipelineStageFlagBits::eTransfer;
// 	}
// 	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
// 		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
// 		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

// 		sourceStage = vk::PipelineStageFlagBits::eTransfer;
// 		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
// 	}
// 	else {
// 		throw std::invalid_argument("unsupported layout transition!");
// 	}

// 	commandBuffer.pipelineBarrier(sourceStage, destinationStage,
// 		vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barriers);

// 	_endSingleTimeCommand(commandBuffer);*/
// }

// void VulkanRenderer::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
// {
// 	/*vk::CommandBuffer cmdBuffer = _beginSingleTimeCommand();


// 	vk::BufferImageCopy region({
// 		0,
// 		0,
// 		0,
// 		vk::ImageSubresourceLayers({
// 			vk::ImageAspectFlagBits::eColor,
// 			(uint32_t)0,
// 			(uint32_t)0,
// 			(uint32_t)1
// 		}),
// 		vk::Offset3D({0, 0, 0}),
// 		vk::Extent3D({ width, height, 1})
// 	});

// 	std::array<vk::BufferImageCopy, 1> regions = { region };
// 	cmdBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, regions);

// 	_endSingleTimeCommand(cmdBuffer);*/
// }
