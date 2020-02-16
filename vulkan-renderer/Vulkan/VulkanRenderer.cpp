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
#include "Drawable.h"
#include "Context.h"
#include "Camera.hpp"
#include "SHLight.h"
#include "Skybox.h"

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

    _camera = new VulkanCamera(&_memoryAllocator);
    _camera->type = VulkanCamera::CameraType::lookat;
    _camera->setPosition(glm::vec3(0, 0, 0));
    _camera->setRotation(glm::vec3(-45, 0, 45));
    _camera->setPerspective(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 10.0f);

    _light = new SHLight(&_memoryAllocator);
    _initSwapchain();
    _initSwapchainImages();
    _initDepthStencilImage();
    _initFramebuffers();
    _initDescriptorPool();
    _initDescriptorSet();
    _initSynchronizations();

    _resourceManager = new ResourceManager(_device, _commandPool, _queue, _graphicsQueueFamilyIndex, _memoryAllocator, _descriptorPool, _gpu);
    _skybox = new Skybox(_resourceManager, _context);
    _skybox->LoadFromDDS("./TestModel/Skybox/env.dds", _device, _descriptorPool);
}

VulkanRenderer::~VulkanRenderer()
{
    vkDeviceWaitIdle(_device);
    vkQueueWaitIdle(_queue);

    delete _skybox;
    delete _window;
    delete _resourceManager;

    _deInitSynchronizations();
    _deInitDescriptorSet();
    _deInitDescriptorPool();
    _deInitFramebuffers();
    _deInitDepthStencilImage();
    _deInitSwapchainImages();
    _deInitSwapchain();

    delete _pipelineManager;
    delete _light;
    delete _camera;

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
    _initDescriptorSet();
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
    return _renderPass;
}

vk::Framebuffer VulkanRenderer::GetActiveFramebuffer()
{
    return _framesData[_activeSwapchainImageId].framebuffer;
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
    return _depthStencilFormat;
}

vk::RenderPass VulkanRenderer::GetRenderPass()
{
    return _renderPass;
}

VulkanCamera * VulkanRenderer::GetCamera()
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

    _camera->UpdateUniformBuffer();
    _light->UpdateUniformBuffer();
}

void VulkanRenderer::_cleanupSwapchain()
{
    _deInitSwapchainImages();
    _deInitDepthStencilImage();
    _deInitFramebuffers();
    _deInitDescriptorPool();
    _deInitDescriptorSet();
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
    RenderPass renderPass(&_device);
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
    renderPass.AddAttachment(colorAttachment);
    renderPass.AddAttachment(depthAttachment);

    _renderPass = renderPass.Get();
    
    for (int i = 0; i < _swapchainImageCount; ++i)
    {

        std::array<vk::ImageView, 2> attachments{};
        attachments[0] = _swapchainImageViews[i];
        attachments[1] = _depthStencilImageView;

        vk::FramebufferCreateInfo createInfo({ {},
                                              _renderPass,
                                              (uint32_t)attachments.size(),
                                              attachments.data(),
                                              _swapchainExtent.width,
                                              _swapchainExtent.height,
                                              uint32_t(1) });
        _framesData[i].framebuffer = _device.createFramebuffer(createInfo);
    }
}

void VulkanRenderer::_deInitFramebuffers()
{
    // TODO: create and destroy should be in the same class.
    _device.destroyRenderPass(_renderPass);
    for (uint32_t i = 0; i < _swapchainImageCount; ++i)
    {
        vkDestroyFramebuffer(_device, _framesData[i].framebuffer, nullptr);
    }
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
                                           static_cast<uint32_t>(20),
                                           static_cast<uint32_t>(poolSizes.size()),
                                           poolSizes.data()});
    _descriptorPool = _device.createDescriptorPool(poolInfo);

    // Init camera uniform buffer descriptor
    // TODO: put the code below to some more appropriate place.
    _camera->createDescriptorSet(_device, _descriptorPool);
    _light->CreateDescriptorSet(_device, _descriptorPool);
}

void VulkanRenderer::_deInitDescriptorPool()
{
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
}

void VulkanRenderer::_initDescriptorSet()
{
    //std::vector<VkDescriptorSetLayout> layouts(_swapchainImages.size(), _descriptorSetLayout);
    //VkDescriptorSetAllocateInfo allocInfo = {};
    //allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    //allocInfo.descriptorPool = _descriptorPool;
    //allocInfo.descriptorSetCount = static_cast<uint32_t>(_swapchainImages.size());
    //allocInfo.pSetLayouts = layouts.data();

    //_descriptorSets.resize(_swapchainImages.size());
    //vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data());

    //for (size_t i = 0; i < _swapchainImages.size(); i++)
    //{
    //    VkDescriptorBufferInfo bufferInfo = {};
    //    bufferInfo.buffer = _uniformBuffers[i];
    //    bufferInfo.offset = 0;
    //    bufferInfo.range = sizeof(UniformBufferObject);

    //    VkDescriptorImageInfo imageInfo = {};
    //    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //    imageInfo.imageView = _textureImageView;
    //    imageInfo.sampler = _textureImageSampler;

    //    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
    //    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //    descriptorWrites[0].dstSet = _descriptorSets[i];
    //    descriptorWrites[0].dstBinding = 0;
    //    descriptorWrites[0].dstArrayElement = 0;
    //    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //    descriptorWrites[0].descriptorCount = 1;
    //    descriptorWrites[0].pBufferInfo = &bufferInfo;
    //    descriptorWrites[0].pImageInfo = nullptr;                // Optional
    //    descriptorWrites[0].pTexelBufferView = nullptr; // Optional

    //    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //    descriptorWrites[1].dstSet = _descriptorSets[i];
    //    descriptorWrites[1].dstBinding = 1;
    //    descriptorWrites[1].dstArrayElement = 0;
    //    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //    descriptorWrites[1].descriptorCount = 1;
    //    descriptorWrites[1].pImageInfo = &imageInfo;
    //    vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    //}
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

void VulkanRenderer::_createCommandBuffers()
{
    for (uint32_t i = 0; i < _swapchainImageCount; ++i)
    {
        std::shared_ptr<Pipeline> pipelineModel = nullptr;
        
        // skybox
        PipelineId skyBoxPipelineId;
        skyBoxPipelineId.type = PipelineType::SKYBOX;
        skyBoxPipelineId.model.primitivePart.info.bits.positionVertexData = 1;
        skyBoxPipelineId.model.primitivePart.info.bits.normalVertexData = 0;
        skyBoxPipelineId.model.primitivePart.info.bits.countTexCoord = 1;
        skyBoxPipelineId.model.primitivePart.info.bits.tangentVertexData = 0;
        skyBoxPipelineId.model.primitivePart.info.bits.countColor = 0;
        std::shared_ptr<Pipeline> pipelineSkybox = _pipelineManager->GetPipeline(skyBoxPipelineId);
        
       
        PipelineId lastPipelineId;
       lastPipelineId.model.primitivePart.info.bits.positionVertexData = 0;

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

       std::array<vk::ClearValue, 2> clearValues{};
       clearValues[0].color.float32[0] = 0.0;
       clearValues[0].color.float32[1] = 0.0;
       clearValues[0].color.float32[2] = 0.0;
       clearValues[0].color.float32[3] = 1.0f;

       clearValues[1].depthStencil.depth = 1.0f;
       clearValues[1].depthStencil.stencil = 0;

       vk::RenderPassBeginInfo renderPassInfo({ _renderPass,
           _framesData[i].framebuffer,
           vk::Rect2D({ vk::Offset2D({ 0, 0 }),
               _swapchainExtent }),
               (uint32_t)clearValues.size(),
           clearValues.data() });

       commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

       // skybox command buffer
       commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineSkybox->GetPipeline());
       commandBuffer.bindVertexBuffers(0, _skybox->m_vertexBuffers.size(), _skybox->m_vertexBuffers.data(), _skybox->m_vertexBufferOffsets.data());
       commandBuffer.bindIndexBuffer(_skybox->m_indexBuffer, 0, vk::IndexType::eUint16);
       commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineSkybox->GetPipelineLayout(), 0, 1, &_camera->descriptorSet, 0, nullptr);
       commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineSkybox->GetPipelineLayout(), 1, 1, &_skybox->m_dsSkybox, 0, nullptr);
       // todo: for test only 
       commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineSkybox->GetPipelineLayout(), 2, 1, &_skybox->m_dsPrefilteredMap, 0, nullptr);
       commandBuffer.drawIndexed(_skybox->m_indexNum, 1, 0, 0, 0);

       // draw drawables
       for (auto it : _drawableMap)
       {
           PipelineId pipelineId = it.first;
           if (lastPipelineId != pipelineId)
           {
               pipelineModel = _pipelineManager->GetPipeline(pipelineId);
               lastPipelineId =  pipelineId;

                commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineModel->GetPipeline());
           
                for (auto drawable : it.second)
                {
                    commandBuffer.bindVertexBuffers(0, drawable->m_vertexBuffers.size(), drawable->m_vertexBuffers.data(), drawable->m_vertexBufferOffsets.data());

                    switch (drawable->m_mesh->m_indexType)
                    {
                    //case 1:
                    //    commandBuffer.bindIndexBuffer(drawable->indexBuffer, 0, vk::IndexType::eUint8EXT);
                    //    break;
                    case 2:
                        commandBuffer.bindIndexBuffer(drawable->m_indexBuffer, 0, vk::IndexType::eUint16);
                        break;
                    case 4:
                        commandBuffer.bindIndexBuffer(drawable->m_indexBuffer, 0, vk::IndexType::eUint32);
                        break;
                    }

                    //pipelineBindPoint, PipelineLayout, firstSet, descriptorSetCount, pDescriptorSets, uint32_t dynamicOffsetCount.
                    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineModel->GetPipelineLayout(), 0, 1, &_camera->descriptorSet, 0, nullptr);
                    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineModel->GetPipelineLayout(), 1, 1, &_light->m_descriptorSet, 0, nullptr);
                    if (drawable->baseColorTexture || drawable->normalTexture)
                    {
                        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineModel->GetPipelineLayout(), 2, 1, &drawable->textureDescriptorSet, 0, nullptr);
                    }

                    uint32_t offset = 0;
                    commandBuffer.pushConstants(pipelineModel->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, offset, sizeof(glm::mat4), reinterpret_cast<void*>(&drawable->m_matrix));
                    offset += sizeof(glm::mat4);
                    commandBuffer.pushConstants(pipelineModel->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, offset, sizeof(glm::mat4), reinterpret_cast<void*>(&drawable->m_normalMatrix));
                    offset += sizeof(glm::mat4);

                    if (pipelineId.model.materialPart.info.bits.baseColorInfo)
                    {
                        commandBuffer.pushConstants(pipelineModel->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, offset, sizeof(glm::vec4), reinterpret_cast<void*>(&drawable->m_material->m_baseColor));
                    }

                    commandBuffer.drawIndexed(drawable->m_mesh->m_indexNum, 1, 0, 0, 0);
                }
           }
       }

       commandBuffer.endRenderPass();
       commandBuffer.end();

       _framesData[i].cmdBuffers.push_back(commandBuffer);
    }
}

void VulkanRenderer::AddRenderNodes(std::vector<std::shared_ptr<Drawable>> drawables)
{
    for (int i = 0; i < drawables.size(); ++i) 
    {
        std::shared_ptr<Drawable> drawable = drawables[i];
        _resourceManager->createNodeResource(drawable);

        PipelineId id;

        id.model.primitivePart.info.bits.positionVertexData = 1;
        id.model.primitivePart.info.bits.normalVertexData = 1;
        id.model.primitivePart.info.bits.tangentVertexData = drawable->m_mesh->m_vertexBits.hasTangent;
        id.model.primitivePart.info.bits.countTexCoord = drawable->m_mesh->m_vertexBits.hasTexCoord0 ? 1 : 0;
        id.model.primitivePart.info.bits.countColor = drawable->m_mesh->m_vertexBits.hasColor;
        id.model.materialPart.info.bits.baseColorInfo = 1;
        id.model.materialPart.info.bits.baseColorMap = drawable->m_material->m_pDiffuseMap != nullptr;
        id.model.materialPart.info.bits.normalMap = drawable->m_material->m_pNormalMap != nullptr;
        id.type = MODEL;
        if (_drawableMap.count(id) == 0) 
        {
            std::vector<std::shared_ptr<Drawable>> drawables_;
            drawables_.push_back(drawable);
            _drawableMap.insert(std::make_pair(id, drawables_));
            auto pipeline = _pipelineManager->GetPipeline(id);
            drawable->m_pPipeline = pipeline;
        }
        else 
        {
            drawable->m_pPipeline = _pipelineManager->GetPipeline(id);
            _drawableMap.at(id).push_back(drawable);
        }
    }

    /*_resourceManager->createDrawableDescriptorSet(drawable);*/
    _createCommandBuffers();
}


// void VulkanRenderer::_initTextureImage()
// {
//     int texWidth, texHeight, texChannels;
//     stbi_uc* pixels = stbi_load("Textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//     VkDeviceSize imageSize = texWidth * texHeight * 4;

//     if (!pixels) {
//         throw std::runtime_error("failed to load texture image!");
//     }

//     VkBuffer stagingBuffer;
//     VmaAllocation stagingBufferMemory;

//     VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
//     stagingBufferInfo.size = imageSize;
//     stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//     VmaAllocationCreateInfo stagingBufferAllocInfo = {};
//     stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
//     vmaCreateBuffer(_memoryAllocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferMemory, nullptr);

//     void* data;
//     vmaMapMemory(_memoryAllocator, stagingBufferMemory, &data);
//     memcpy(data, pixels, static_cast<size_t>(imageSize));
//     vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

//     stbi_image_free(pixels);

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

//     transitionImageLayout(_textureImage, vk::Format::eR8G8B8A8Unorm,
//         vk::ImageLayout::eUndefined , vk::ImageLayout::eTransferDstOptimal);
//     copyBufferToImage(stagingBuffer, _textureImage, 
//         static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
//     transitionImageLayout(_textureImage, vk::Format::eR8G8B8A8Unorm,
//         vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

//     vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);
// }

// void VulkanRenderer::_deInitTextureImage()
// {
//     vmaDestroyImage(_memoryAllocator, _textureImage, _textureImageMemory);
// }

// void VulkanRenderer::_initTextureImageView()
// {
//     vk::ImageViewCreateInfo createInfo({
//         {},
//         _textureImage,
//         vk::ImageViewType::e2D,
//         vk::Format::eR8G8B8A8Unorm,
//         vk::ComponentMapping({
//             vk::ComponentSwizzle::eR,
//             vk::ComponentSwizzle::eG,
//             vk::ComponentSwizzle::eB,
//             vk::ComponentSwizzle::eA
//         }),
//         vk::ImageSubresourceRange({
//             vk::ImageAspectFlagBits::eColor,
//             0,
//             1,
//             0,
//             1
//         })
//     });
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
//     /*vk::CommandBuffer commandBuffer = _beginSingleTimeCommand();

//     vk::ImageMemoryBarrier barrier({
//         {},
//         {},
//         oldLayout,
//         newLayout,
//         {},
//         {},
//         _textureImage,
//         vk::ImageSubresourceRange({
//         vk::ImageAspectFlagBits::eColor,
//         (uint32_t)0,
//         (uint32_t)1,
//         (uint32_t)0,
//         (uint32_t)1,
//     })
//     });

//     std::array<vk::ImageMemoryBarrier, 1> barriers = { barrier };


//     vk::PipelineStageFlagBits sourceStage;
//     vk::PipelineStageFlagBits destinationStage;

//     if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
//         barrier.srcAccessMask = {};
//         barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

//         sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
//         destinationStage = vk::PipelineStageFlagBits::eTransfer;
//     }
//     else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
//         barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
//         barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

//         sourceStage = vk::PipelineStageFlagBits::eTransfer;
//         destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
//     }
//     else {
//         throw std::invalid_argument("unsupported layout transition!");
//     }

//     commandBuffer.pipelineBarrier(sourceStage, destinationStage,
//         vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barriers);

//     _endSingleTimeCommand(commandBuffer);*/
// }

// void VulkanRenderer::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
// {
//     /*vk::CommandBuffer cmdBuffer = _beginSingleTimeCommand();


//     vk::BufferImageCopy region({
//         0,
//         0,
//         0,
//         vk::ImageSubresourceLayers({
//             vk::ImageAspectFlagBits::eColor,
//             (uint32_t)0,
//             (uint32_t)0,
//             (uint32_t)1
//         }),
//         vk::Offset3D({0, 0, 0}),
//         vk::Extent3D({ width, height, 1})
//     });

//     std::array<vk::BufferImageCopy, 1> regions = { region };
//     cmdBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, regions);

//     _endSingleTimeCommand(cmdBuffer);*/
// }
