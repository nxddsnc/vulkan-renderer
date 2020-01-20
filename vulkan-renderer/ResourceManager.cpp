#include "ResourceManager.h"
#include "Drawable.h"
#include "MyMesh.h"
#include "MyMaterial.h"
#include "MyTexture.h"
#include "MyImage.h"

ResourceManager::ResourceManager(vk::Device &device, vk::CommandPool &commandPool, vk::Queue &graphicsQueue,
    uint32_t graphicsQueueFamilyIndex, VmaAllocator memoryAllocator, vk::DescriptorPool &descriptorPool, vk::PhysicalDevice &gpu)
{
    _device         = device;
    _commandPool    = commandPool;
    _graphicsQueue = graphicsQueue;
    _graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    _memoryAllocator = memoryAllocator;
    _descriptorPool = descriptorPool;
    _gpu = gpu;
}

ResourceManager::~ResourceManager()
{
    for (auto drawable : _nodes)
    {
        for (int i = 0; i < drawable->vertexBuffers.size(); ++i)
        {
            vmaDestroyBuffer(_memoryAllocator, drawable->vertexBuffers[i], drawable->vertexBufferMemorys[i]);
        }
        vmaDestroyBuffer(_memoryAllocator, drawable->indexBuffer, drawable->indexBufferMemory);
        if (drawable->baseColorTexture)
        {
            vmaDestroyImage(_memoryAllocator, drawable->baseColorTexture->image, drawable->baseColorTexture->imageMemory);
            _device.destroyImageView(drawable->baseColorTexture->imageView);
            _device.destroySampler(drawable->baseColorTexture->imageSampler);
        }
    }
}

void ResourceManager::createNodeResource(std::shared_ptr<Drawable> drawable)
{
    _createVertexBuffers(drawable);
    _createIndexBuffer(drawable);
    _createTextures(drawable);
    _nodes.push_back(drawable);
}

vk::CommandBuffer ResourceManager::_beginSingleTimeCommand()
{
    vk::CommandBufferAllocateInfo allocInfo({
        _commandPool,
        vk::CommandBufferLevel::ePrimary,
        1
    });
    auto commandBuffers = _device.allocateCommandBuffers(allocInfo);

    vk::CommandBufferBeginInfo beginInfo({
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
        {}
    });
    commandBuffers[0].begin(&beginInfo);
    return commandBuffers[0];
}

void ResourceManager::_endSingleTimeCommand(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo({
        {},
        {},
        {},
        (uint32_t)1,
        &commandBuffer,
        {},
        {}
    });
    _graphicsQueue.submit((uint32_t)1, &submitInfo, nullptr);
    _graphicsQueue.waitIdle();
    _device.freeCommandBuffers(_commandPool, 1, &commandBuffer);
}

void ResourceManager::_copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
    vk::CommandBuffer cmdBuffer = _beginSingleTimeCommand();


    vk::BufferImageCopy region({
        0,
        0,
        0,
        vk::ImageSubresourceLayers({
        vk::ImageAspectFlagBits::eColor,
        (uint32_t)0,
        (uint32_t)0,
        (uint32_t)1
    }),
        vk::Offset3D({ 0, 0, 0 }),
        vk::Extent3D({ width, height, 1 })
    });

    std::array<vk::BufferImageCopy, 1> regions = { region };
    cmdBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, regions);

    _endSingleTimeCommand(cmdBuffer);
}

void ResourceManager::_copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    vk::CommandBuffer commandBuffer = _beginSingleTimeCommand();

    vk::BufferCopy copyRegion({
        0,
        0,
        size
    });
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

    _endSingleTimeCommand(commandBuffer);
}

void ResourceManager::_createVertexBuffer(std::shared_ptr<Drawable> drawable, vk::DeviceSize size, void *data_)
{
    vk::Buffer buffer;
    VmaAllocation bufferMemory;
    vk::BufferCreateInfo bufferInfo({
        {},
        size,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        1,
        &_graphicsQueueFamilyIndex
    });

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateBuffer(_memoryAllocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocInfo, reinterpret_cast<VkBuffer*>(&buffer), &bufferMemory, nullptr);

    vk::Buffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    vk::BufferCreateInfo stagingBufferInfo({
        {},
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        1,
        &_graphicsQueueFamilyIndex
    });
    VmaAllocationCreateInfo stagingBufferAllocInfo = {};
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaCreateBuffer(_memoryAllocator, reinterpret_cast<VkBufferCreateInfo*>(&stagingBufferInfo), &stagingBufferAllocInfo, reinterpret_cast<VkBuffer*>(&stagingBuffer), &stagingBufferMemory, nullptr);

    void* data;
    vmaMapMemory(_memoryAllocator, stagingBufferMemory, &data);
    memcpy(data, data_, (size_t)size);
    vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

    _copyBuffer(stagingBuffer, buffer, size);

    vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);

    drawable->vertexBuffers.push_back(std::move(buffer));
    drawable->vertexBufferMemorys.push_back(std::move(bufferMemory));
    drawable->vertexBufferOffsets.push_back(vk::DeviceSize(0));
}

void ResourceManager::_createVertexBuffers(std::shared_ptr<Drawable> drawable)
{
    vk::DeviceSize size = sizeof(drawable->mesh->m_positions[0]) * drawable->mesh->m_positions.size();
    _createVertexBuffer(drawable, size, reinterpret_cast<void*>(drawable->mesh->m_positions.data()));

    size = sizeof(drawable->mesh->m_normals[0]) * drawable->mesh->m_normals.size();
    _createVertexBuffer(drawable, size, reinterpret_cast<void*>(drawable->mesh->m_normals.data()));

    if (drawable->mesh->m_texCoords0.size() > 0)
    {
        size = sizeof(drawable->mesh->m_texCoords0[0]) * drawable->mesh->m_texCoords0.size();
        _createVertexBuffer(drawable, size, reinterpret_cast<void*>(drawable->mesh->m_texCoords0.data()));
    }
    if (drawable->mesh->m_tangents.size() > 0)
    {
        size = sizeof(drawable->mesh->m_tangents[0]) * drawable->mesh->m_tangents.size();
        _createVertexBuffer(drawable, size, reinterpret_cast<void*>(drawable->mesh->m_tangents.data()));
    }
}

void ResourceManager::_createIndexBuffer(std::shared_ptr<Drawable> drawable)
{
    VkDeviceSize size = drawable->mesh->m_indexNum * drawable->mesh->getIndexSize();

    vk::BufferCreateInfo bufferInfo({
        {},
        size,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        1,
        &_graphicsQueueFamilyIndex
    });
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateBuffer(_memoryAllocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocInfo, reinterpret_cast<VkBuffer*>(&drawable->indexBuffer), &drawable->indexBufferMemory, nullptr);

    vk::Buffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    vk::BufferCreateInfo stagingBufferInfo({
        {},
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        1,
        &_graphicsQueueFamilyIndex
    });
    VmaAllocationCreateInfo stagingBufferAllocInfo = {};
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaCreateBuffer(_memoryAllocator, reinterpret_cast<VkBufferCreateInfo*>(&stagingBufferInfo), &stagingBufferAllocInfo, reinterpret_cast<VkBuffer*>(&stagingBuffer), &stagingBufferMemory, nullptr);

    void* data;
    vmaMapMemory(_memoryAllocator, stagingBufferMemory, &data);
    memcpy(data, drawable->mesh->m_indices, (size_t)size);
    vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

    _copyBuffer(stagingBuffer, drawable->indexBuffer, size);

    vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);
}

void ResourceManager::_transitionImageLayout(vk::Image &image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::CommandBuffer commandBuffer = _beginSingleTimeCommand();

    vk::ImageMemoryBarrier barrier({
        {},
        {},
        oldLayout,
        newLayout,
        {},
        {},
        image,
        vk::ImageSubresourceRange({
        vk::ImageAspectFlagBits::eColor,
        (uint32_t)0,
        (uint32_t)1,
        (uint32_t)0,
        (uint32_t)1,
    })
    });

    std::array<vk::ImageMemoryBarrier, 1> barriers = { barrier };


    vk::PipelineStageFlagBits sourceStage;
    vk::PipelineStageFlagBits destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage,
        vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barriers);

    _endSingleTimeCommand(commandBuffer);
}

std::shared_ptr<VulkanTexture> ResourceManager::_createCombinedTexture(std::shared_ptr<MyTexture> texture)
{
    std::shared_ptr<MyImage> myImage = texture->m_pImage;
    std::shared_ptr<VulkanTexture> vulkanTexture = std::make_shared<VulkanTexture>();

    vk::Format imageFormat = vk::Format::eR8G8B8A8Unorm;
    //if (texture->m_pImage->m_channels == 3)
    //{
    //    imageFormat = vk::Format::eR8G8B8Unorm;
    //}

    auto properties = _gpu.getFormatProperties(imageFormat);
    // create image
    vk::ImageCreateInfo imageCreateInfo( {},
                                          vk::ImageType::e2D,
                                          imageFormat,
                                          vk::Extent3D({
                                              static_cast<uint32_t>(myImage->m_width),
                                              static_cast<uint32_t>(myImage->m_height),
                                              1
                                          }),
                                          1,
                                          1,
                                          vk::SampleCountFlagBits::e1,
                                          vk::ImageTiling::eOptimal,
                                          vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
                                          vk::SharingMode::eExclusive,
                                          1,
                                          &_graphicsQueueFamilyIndex,
                                          vk::ImageLayout::eUndefined);
    VmaAllocationCreateInfo allocationCreateInfo = {};

    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateImage(_memoryAllocator, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo,
        reinterpret_cast<VkImage*>(&(vulkanTexture->image)), &(vulkanTexture->imageMemory), nullptr);

    // create image view
    vk::ImageViewCreateInfo imageViewCreateInfo({
        {},
        vulkanTexture->image,
        vk::ImageViewType::e2D,
        imageFormat,
        vk::ComponentMapping({
            vk::ComponentSwizzle::eR,
            vk::ComponentSwizzle::eG,
            vk::ComponentSwizzle::eB,
            {}
        }),
        vk::ImageSubresourceRange({
            vk::ImageAspectFlagBits::eColor,
            0,
            1,
            0,
            1
        })
    });
    vulkanTexture->imageView = _device.createImageView(imageViewCreateInfo);

    // create image sampler
    vk::SamplerCreateInfo createInfo(
        {},
        vk::Filter::eLinear,
        vk::Filter::eLinear,
        vk::SamplerMipmapMode::eLinear,
        vk::SamplerAddressMode::eRepeat,
        vk::SamplerAddressMode::eRepeat,
        vk::SamplerAddressMode::eRepeat,
        0.0,
        vk::Bool32(true),
        16.0,
        vk::Bool32(false),
        vk::CompareOp::eAlways,
        0.0,
        0.0,
        vk::BorderColor::eFloatOpaqueWhite,
        vk::Bool32(false)
    );

    vulkanTexture->imageSampler = _device.createSampler(createInfo);

    // transfer data from RAM to GPU memory
    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    stagingBufferInfo.size = texture->m_pImage->m_width * texture->m_pImage->m_height * texture->m_pImage->m_channels;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo stagingBufferAllocInfo = {};
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaCreateBuffer(_memoryAllocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferMemory, nullptr);

    void* data;
    vmaMapMemory(_memoryAllocator, stagingBufferMemory, &data);
    memcpy(data, texture->m_pImage->m_data, static_cast<size_t>(stagingBufferInfo.size));
    vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

    _transitionImageLayout(vulkanTexture->image, imageFormat,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    _copyBufferToImage(stagingBuffer, vulkanTexture->image,
        static_cast<uint32_t>(texture->m_pImage->m_width), static_cast<uint32_t>(texture->m_pImage->m_height));
    _transitionImageLayout(vulkanTexture->image, imageFormat,
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);

    return vulkanTexture;
}

void ResourceManager::_createTextures(std::shared_ptr<Drawable> drawable)
{
    if (drawable->material->m_pDiffuseMap)
    {
        drawable->baseColorTexture = _createCombinedTexture(drawable->material->m_pDiffuseMap);
    }

    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSetLayoutBinding textureBinding({ 0,
                                                   vk::DescriptorType::eCombinedImageSampler,
                                                   1,
                                                   vk::ShaderStageFlagBits::eFragment,
                                                   {} });

    vk::DescriptorSetLayoutCreateInfo layoutInfo({ {},
        1,
        &textureBinding });
    descriptorSetLayout = _device.createDescriptorSetLayout(layoutInfo);

    vk::DescriptorSetAllocateInfo allocInfo({
        _descriptorPool,
        1,
        &descriptorSetLayout
    });
    
    _device.allocateDescriptorSets(&allocInfo, &drawable->textureDescriptorSet);


    vk::DescriptorImageInfo imageInfo({ drawable->baseColorTexture->imageSampler,
                                        drawable->baseColorTexture->imageView,
                                        vk::ImageLayout::eShaderReadOnlyOptimal});


    vk::WriteDescriptorSet descriptorWrite( drawable->textureDescriptorSet,
                                            uint32_t(0),
                                            uint32_t(0),
                                            uint32_t(1),
                                            vk::DescriptorType::eCombinedImageSampler,
                                            &imageInfo,
                                            {},
                                            {} );
    _device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

    _device.destroyDescriptorSetLayout(descriptorSetLayout);
}