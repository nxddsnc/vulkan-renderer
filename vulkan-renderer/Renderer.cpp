#include "Renderer.h"
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include "BUILD_OPTIONS.h"
#include "Shared.h"
#include "Window.h"
#include <array>
#include "Scene.h"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Renderer::Renderer(Window *window)
{
	_window				= window;
	_swapchainExtent	= { WIDTH, HEIGHT };
	_context			= VulkanContext(window);

	_gpu				= _context.GetPhysicalDevice();
	_device			    = _context.GetLogicalDevice();
	_queue			    = _context.GetDeviceQueue();
	_surface            = _context.GetSuface();
	_commandPool	    = _context.GetCommandPool();
	//present_queue = vulkan_context.getPresentQueue();
	//compute_queue = vulkan_context.getComputeQueue();
	//graphics_command_pool = vulkan_context.getGraphicsCommandPool();
	//compute_command_pool = vulkan_context.getComputeCommandPool();
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _gpu;
    allocatorInfo.device = _device;

    vmaCreateAllocator(&allocatorInfo, &_memoryAllocator);

	_initSwapchain();
	_initSwapchainImages();
	//_initDepthStencilImage();
	_initRenderPass();
	_initDescriptorSetLayout();
	_initGraphicsPipeline();
	_initFramebuffers();
	//_initTextureImage();
	_initVertexBuffer();
    _initIndexBuffer();
	_initUniformBuffers();
	_initDescriptorPool();
	_initDescriptorSet();
	_initCommandBuffers();
	_initSynchronizations();
}

Renderer::~Renderer()
{
    vkDeviceWaitIdle(_device);
	vkQueueWaitIdle(_queue);
	delete _window;
	_deInitSynchronizations();
	_deInitCommandBuffers();
	_deInitDescriptorSet();
	_deInitDescriptorPool(); 
	_deInitUniformBuffer();
    _deInitIndexBuffer();
	_deInitVertexBuffer();
	//_deInitTextureImage();
	_deInitFramebuffer();
	_deInitGraphicPipeline();
	_deInitDescriptorSetLayout();
	_deInitRenderPass();
	//_deInitDepthStencilImage();
	_deInitSwapchainImages();
	_deInitSwapchain();

    vmaDestroyAllocator(_memoryAllocator);
}

bool Renderer::Run()
{
	if (nullptr != _window) 
	{
		return _window->Update();
	}
	return true;
}

void Renderer::Resize(int width, int height)
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
	_initUniformBuffers();
	_initDescriptorPool();
	_initDescriptorSet(); 
	_initCommandBuffers();
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

void Renderer::DrawFrame()
{
	_beginRender();
	
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffers[_activeSwapchainImageId];

	std::vector<VkSemaphore> signalSemaphores{ _renderFinishedSemaphores[_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);

	_endRender(signalSemaphores);
}

void Renderer::_beginRender()
{
	vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &_activeSwapchainImageId);
	_updateUniformBuffer();
	//vkWaitForFences(_device, 1, &_swapchainImageAvailable, VK_TRUE, UINT64_MAX);
	//vkResetFences(_device, 1, &_swapchainImageAvailable);
	//vkQueueWaitIdle(_queue);
}

void Renderer::_endRender(std::vector<VkSemaphore> waitSemaphores)
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
	_currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::_updateUniformBuffer()
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), _swapchainExtent.width / (float)_swapchainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	void* data;
	vmaMapMemory(_memoryAllocator, _uniformBuffersMemory[_activeSwapchainImageId], &data);
	memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(_memoryAllocator, _uniformBuffersMemory[_activeSwapchainImageId]);
}

void Renderer::_cleanupSwapchain()
{
    for (size_t i = 0; i < _framebuffers.size(); ++i) {
        vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
    }

    vkFreeCommandBuffers(_device, _commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());

    vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
    vkDestroyRenderPass(_device, _renderPass, nullptr);

    for (size_t i = 0; i < _swapchainImageViews.size(); ++i) {
        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
    }
	_deInitDepthStencilImage();
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

	_deInitUniformBuffer();
	_deInitDescriptorPool();
	_deInitDescriptorSet();
}

VkShaderModule Renderer::_createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkShaderModule shaderModule;
	vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule);
	return shaderModule;
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

    //VULKAN_HPP_NAMESPACE::SwapchainCreateFlagsKHR flags_ = {},
    //    VULKAN_HPP_NAMESPACE::SurfaceKHR surface_ = {},
    //    uint32_t minImageCount_ = {},
    //    VULKAN_HPP_NAMESPACE::Format imageFormat_ = {},
    //    VULKAN_HPP_NAMESPACE::ColorSpaceKHR imageColorSpace_ = {},
    //    VULKAN_HPP_NAMESPACE::Extent2D imageExtent_ = {},
    //    uint32_t imageArrayLayers_ = {},
    //    VULKAN_HPP_NAMESPACE::ImageUsageFlags imageUsage_ = {},
    //    VULKAN_HPP_NAMESPACE::SharingMode imageSharingMode_ = {},
    //    uint32_t queueFamilyIndexCount_ = {},
    //    const uint32_t* pQueueFamilyIndices_ = {},
    //    VULKAN_HPP_NAMESPACE::SurfaceTransformFlagBitsKHR preTransform_ = {},
    //    VULKAN_HPP_NAMESPACE::CompositeAlphaFlagBitsKHR compositeAlpha_ = {},
    //    VULKAN_HPP_NAMESPACE::PresentModeKHR presentMode_ = {},
    //    VULKAN_HPP_NAMESPACE::Bool32 clipped_ = {},
    //    VULKAN_HPP_NAMESPACE::SwapchainKHR oldSwapchain_ = {}

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
}
void Renderer::_deInitSwapchain()
{
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

void Renderer::_initSwapchainImages()
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
				0,
				0,
				1
			})
		});
		_swapchainImageViews[i] = _device.createImageView(createInfo);
	}
}
void Renderer::_deInitSwapchainImages()
{
	for (uint32_t i = 0; i < _swapchainImageCount; ++i)
	{
		_device.destroyImageView(_swapchainImageViews[i]);
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
	vkAllocateMemory(_device, &allocateInfo, nullptr, &_depthStencilImageMemory);
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
	vkDestroyImage(_device, _depthStencilImage, nullptr);
	vkFreeMemory(_device, _depthStencilImageMemory, nullptr);
}

void Renderer::_initRenderPass()
{
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

	vk::AttachmentDescription depthAttachment({
		{},
		_depthStencilFormat,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eStore,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	});

	vk::AttachmentReference subpassDepthStencilAttachment({
		{},
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	});

	std::array<vk::AttachmentReference, 1> subpassColorAttachments{};
	subpassColorAttachments[0].attachment = 1;
	subpassColorAttachments[0].layout = vk::ImageLayout::eColorAttachmentOptimal;

	std::array<vk::SubpassDescription, 1> subpasses{};
	subpasses[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpasses[0].inputAttachmentCount = 0;
	subpasses[0].pInputAttachments = VK_NULL_HANDLE;
	subpasses[0].colorAttachmentCount = subpassColorAttachments.size();
	subpasses[0].pColorAttachments = subpassColorAttachments.data();
	subpasses[0].pDepthStencilAttachment = &subpassDepthStencilAttachment;

	// TODO: look into the documentation of subpass and subpass dependencies.
	vk::SubpassDependency dependency({
		VK_SUBPASS_EXTERNAL,
		0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		{}
	});

	std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	vk::RenderPassCreateInfo createInfo({
		{},
		2,
		attachments.data(),
		subpasses.size(),
		subpasses.data(),
		1,
		dependency
	});

	_renderPass = _device.createRenderPass(createInfo);
}

void Renderer::_deInitRenderPass()
{
	_device.destroyRenderPass(_renderPass);
}

void Renderer::_initDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout);
}

void Renderer::_deInitDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);
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

	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

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
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.lineWidth = 1.0;

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
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
	vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout);

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
	depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	//depthStencilStateCreateInfo.flags;
	depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
	//depthStencilStateCreateInfo.depthCompareOp;
	//depthStencilStateCreateInfo.depthBoundsTestEnable;
	//depthStencilStateCreateInfo.stencilTestEnable;
	//depthStencilStateCreateInfo.front;
	//depthStencilStateCreateInfo.back;
	//depthStencilStateCreateInfo.minDepthBounds;
	//depthStencilStateCreateInfo.maxDepthBounds;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline);

	vkDestroyShaderModule(_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}

void Renderer::_deInitGraphicPipeline()
{
	vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
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
		vkCreateFramebuffer(_device, &createInfo, nullptr, &_framebuffers[i]);
	}
}

void Renderer::_deInitFramebuffer()
{
	for (uint32_t i = 0; i < _swapchainImageCount; ++i)
	{
		vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
	}
}

void Renderer::_initTextureImage()
{
	/*int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("Textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(_device, imageSize, &_gpuMemoryProperties, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(_device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(_device, stagingBufferMemory);

	stbi_image_free(pixels);

	createImage(_device, &_gpuMemoryProperties, texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _textureImage, _textureImageMemory);*/
}

void Renderer::_deInitTextureImage()
{

}

void Renderer::_copyBuffer(vk::CommandPool &commandPool, vk::Buffer srcBuffer,
    vk::Buffer dstBuffer, vk::DeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_queue);

    vkFreeCommandBuffers(_device, commandPool, 1, &commandBuffer);
}

void Renderer::_initVertexBuffer()
{
	const std::vector<Vertex> vertices = {
		{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } },
		{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f } }
	};

	VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateBuffer(_memoryAllocator, &bufferInfo, &allocInfo, &_vertexBuffer, &_vertexBufferMemory, nullptr);

	VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    stagingBufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo stagingBufferAllocInfo = {};
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaCreateBuffer(_memoryAllocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferMemory, nullptr);

    void* data;
    vmaMapMemory(_memoryAllocator, stagingBufferMemory, &data);
    memcpy(data, vertices.data(), (size_t)size);
    vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

	_copyBuffer(_commandPool, stagingBuffer, _vertexBuffer, size);

    vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);
}

void Renderer::_deInitVertexBuffer()
{
    vmaDestroyBuffer(_memoryAllocator, _vertexBuffer, _vertexBufferMemory);
}

void Renderer::_initIndexBuffer()
{
    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    VkDeviceSize size = sizeof(indices[0]) * indices.size();

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateBuffer(_memoryAllocator, &bufferInfo, &allocInfo, &_indexBuffer, &_indexBufferMemory, nullptr);

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    stagingBufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo stagingBufferAllocInfo = {};
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaCreateBuffer(_memoryAllocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferMemory, nullptr);

    void* data;
    vmaMapMemory(_memoryAllocator, stagingBufferMemory, &data);
    memcpy(data, indices.data(), (size_t)size);
    vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

    _copyBuffer(_commandPool, stagingBuffer, _indexBuffer, size);

    vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);
}

void Renderer::_deInitIndexBuffer()
{
    vmaDestroyBuffer(_memoryAllocator, _indexBuffer, _indexBufferMemory);
}

void Renderer::_initUniformBuffers()
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

void Renderer::_deInitUniformBuffer()
{
	for (size_t i = 0; i < _swapchainImages.size(); i++)
	{
        vmaDestroyBuffer(_memoryAllocator, _uniformBuffers[i], _uniformBuffersMemory[i]);
	}
}

void Renderer::_initDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(_swapchainImages.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<uint32_t>(_swapchainImages.size());;
	vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool);
}

void Renderer::_deInitDescriptorPool()
{
	vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
}

void Renderer::_initDescriptorSet()
{
	std::vector<VkDescriptorSetLayout> layouts(_swapchainImages.size(), _descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(_swapchainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	_descriptorSets.resize(_swapchainImages.size());
	vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data());

	for (size_t i = 0; i < _swapchainImages.size(); i++) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = _uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);
	
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = _descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional
		vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);
	}
}

void Renderer::_deInitDescriptorSet()
{
}

void Renderer::_initCommandBuffers()
{
	_commandBuffers.resize(_swapchainImageCount);
	for (uint32_t i = 0; i < _commandBuffers.size(); ++i)
	{
		VkCommandBuffer commandBuffer = {};
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = _commandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1;
		vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, &commandBuffer);
		_commandBuffers[i]= commandBuffer;
	}

	for (size_t i = 0; i < _commandBuffers.size(); ++i) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		vkBeginCommandBuffer(_commandBuffers[i], &beginInfo);
		
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = _renderPass;
		renderPassInfo.framebuffer = _framebuffers[i];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = _swapchainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].depthStencil.depth = 0.0f;
        clearValues[0].depthStencil.stencil = 0;

        clearValues[1].color.float32[0] = 0.0;
        clearValues[1].color.float32[1] = 0.0;
        clearValues[1].color.float32[2] = 0.0;
        clearValues[1].color.float32[3] = 1.0f;
		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);
		
		VkBuffer vertexBuffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(_commandBuffers[i], _indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		
		vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[i], 0, nullptr);

		vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(6), 1, 0, 0, 0);

		vkCmdEndRenderPass(_commandBuffers[i]);
		vkEndCommandBuffer(_commandBuffers[i]);
	}
}

void Renderer::_deInitCommandBuffers()
{
	// The command buffers are allocated from the commandBufferPool, so no need to clean up the memory.
}

void Renderer::_initSynchronizations()
{
	VkFenceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(_device, &createInfo, nullptr, &_swapchainImageAvailable);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]);
		vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]);
	}

}

void Renderer::_deInitSynchronizations()
{
	vkDestroyFence(_device, _swapchainImageAvailable, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
	}
}