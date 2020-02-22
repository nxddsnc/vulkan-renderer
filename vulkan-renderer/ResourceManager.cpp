#include "ResourceManager.h"
#include "Drawable.h"
#include "MyMesh.h"
#include "MyMaterial.h"
#include "MyTexture.h"
#include "MyImage.h"
#include <algorithm>

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
        for (int i = 0; i < drawable->m_vertexBuffers.size(); ++i)
        {
            vmaDestroyBuffer(_memoryAllocator, drawable->m_vertexBuffers[i], drawable->m_vertexBufferMemorys[i]);
        }
        vmaDestroyBuffer(_memoryAllocator, drawable->m_indexBuffer, drawable->m_indexBufferMemory);
        if (drawable->baseColorTexture)
        {
            if (drawable->baseColorTexture)
            {
                vmaDestroyImage(_memoryAllocator, drawable->baseColorTexture->image, drawable->baseColorTexture->imageMemory);
                _device.destroyImageView(drawable->baseColorTexture->imageView);
                _device.destroySampler(drawable->baseColorTexture->imageSampler);
            }
            if (drawable->normalTexture)
            {
                vmaDestroyImage(_memoryAllocator, drawable->normalTexture->image, drawable->normalTexture->imageMemory);
                _device.destroyImageView(drawable->normalTexture->imageView);
                _device.destroySampler(drawable->normalTexture->imageSampler);
            }
        }

    }
}

void ResourceManager::InitVulkanBuffers(std::shared_ptr<Drawable> drawable)
{
    CreateVertexBuffers(drawable);
    CreateIndexBuffer(drawable->m_mesh, drawable->m_indexBuffer, drawable->m_indexBufferMemory);
}

void ResourceManager::InitVulkanResource(std::shared_ptr<Drawable> drawable)
{
    InitVulkanBuffers(drawable);
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

void ResourceManager::_copyBufferToImage(vk::Buffer buffer, vk::Image image, std::shared_ptr<MyImage> myImage)
{
    vk::CommandBuffer cmdBuffer = _beginSingleTimeCommand();

    vk::DeviceSize offset = 0;
    std::vector<vk::BufferImageCopy> regions;
    for (int layer = 0; layer < myImage->m_layerCount; ++layer)
    {
        for (int level = 0; level < myImage->m_mipmapCount; ++level)
        {
            vk::BufferImageCopy region(
                offset,
                0,
                0,
                vk::ImageSubresourceLayers(
                    vk::ImageAspectFlagBits::eColor,
                    (uint32_t)level,
                    (uint32_t)layer,
                    (uint32_t)1
                ),
                vk::Offset3D(0, 0, 0),
                vk::Extent3D(std::max(1, int(myImage->m_width / pow(2, level))), std::max(1, int(myImage->m_height / pow(2, level))), 1)
            );
            regions.push_back(region);

            if (myImage->m_bCompressed)
            {
                offset += std::max(1, int((myImage->m_width + 3) / 4 * (myImage->m_height + 3) / 4 *
                    myImage->m_channels * myImage->m_blockSize / pow(4, level)));
            }
            else
            {
                offset += std::max(1, int(myImage->m_width * myImage->m_height * myImage->m_channels * myImage->m_blockSize / pow(4, level)));
            }
        }
    }

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

void ResourceManager::InitVertexBuffer(vk::DeviceSize size, void *data_, vk::Buffer &buffer, VmaAllocation &bufferMemory, vk::DeviceSize &bufferOffset)
{
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

    bufferOffset = 0;
}

void ResourceManager::CreateVertexBuffers(std::shared_ptr<Drawable> drawable)
{
    vk::DeviceSize size = sizeof(drawable->m_mesh->m_positions[0]) * drawable->m_mesh->m_positions.size();
    vk::Buffer positionBuffer;
    VmaAllocation positionBufferMemory;
    vk::DeviceSize positionBufferOffset;
    InitVertexBuffer(size, reinterpret_cast<void*>(drawable->m_mesh->m_positions.data()), positionBuffer, positionBufferMemory, positionBufferOffset);
    drawable->m_vertexBuffers.push_back(std::move(positionBuffer));
    drawable->m_vertexBufferMemorys.push_back(std::move(positionBufferMemory));
    drawable->m_vertexBufferOffsets.push_back(std::move(positionBufferOffset));

    if (drawable->m_mesh->m_normals.size() > 0)
    {
        vk::Buffer normalBuffer;
        VmaAllocation normalBufferMemory;
        vk::DeviceSize normalBufferOffset;
        size = sizeof(drawable->m_mesh->m_normals[0]) * drawable->m_mesh->m_normals.size();
        InitVertexBuffer(size, reinterpret_cast<void*>(drawable->m_mesh->m_normals.data()), normalBuffer, normalBufferMemory, normalBufferOffset);
        drawable->m_vertexBuffers.push_back(std::move(normalBuffer));
        drawable->m_vertexBufferMemorys.push_back(std::move(normalBufferMemory));
        drawable->m_vertexBufferOffsets.push_back(std::move(normalBufferOffset));
    }

    if (drawable->m_mesh->m_texCoords0.size() > 0)
    {
        vk::Buffer uvBuffer;
        VmaAllocation uvBufferMemory;
        vk::DeviceSize uvBufferOffset;
        size = sizeof(drawable->m_mesh->m_texCoords0[0]) * drawable->m_mesh->m_texCoords0.size();
        InitVertexBuffer(size, reinterpret_cast<void*>(drawable->m_mesh->m_texCoords0.data()), uvBuffer, uvBufferMemory, uvBufferOffset);
        drawable->m_vertexBuffers.push_back(std::move(uvBuffer));
        drawable->m_vertexBufferMemorys.push_back(std::move(uvBufferMemory));
        drawable->m_vertexBufferOffsets.push_back(std::move(uvBufferOffset));
    }
    if (drawable->m_mesh->m_tangents.size() > 0)
    {
        vk::Buffer tangentBuffer;
        VmaAllocation tangentBufferMemory;
        vk::DeviceSize tangentBufferOffset;
        size = sizeof(drawable->m_mesh->m_tangents[0]) * drawable->m_mesh->m_tangents.size();
        InitVertexBuffer(size, reinterpret_cast<void*>(drawable->m_mesh->m_tangents.data()), tangentBuffer, tangentBufferMemory, tangentBufferOffset);
        drawable->m_vertexBuffers.push_back(std::move(tangentBuffer));
        drawable->m_vertexBufferMemorys.push_back(std::move(tangentBufferMemory));
        drawable->m_vertexBufferOffsets.push_back(std::move(tangentBufferOffset));
    }
    if (drawable->m_mesh->m_colors.size() > 0)
    {
        vk::Buffer colorBuffer;
        VmaAllocation colorBufferMemory;
        vk::DeviceSize colorBufferOffset;
        size = sizeof(drawable->m_mesh->m_colors[0]) * drawable->m_mesh->m_colors.size();
        InitVertexBuffer(size, reinterpret_cast<void*>(drawable->m_mesh->m_colors.data()), colorBuffer, colorBufferMemory, colorBufferOffset);
        drawable->m_vertexBuffers.push_back(std::move(colorBuffer));
        drawable->m_vertexBufferMemorys.push_back(std::move(colorBufferMemory));
        drawable->m_vertexBufferOffsets.push_back(std::move(colorBufferOffset));
    }
}

void ResourceManager::CreateIndexBuffer(std::shared_ptr<MyMesh> mesh, vk::Buffer &buffer, VmaAllocation &bufferMemory)
{
    VkDeviceSize size = mesh->m_indexNum * mesh->getIndexSize();

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
    memcpy(data, mesh->m_indices, (size_t)size);
    vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

    _copyBuffer(stagingBuffer, buffer, size);

    vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);
}

void ResourceManager::SetImageLayout(vk::CommandBuffer& commandBuffer, vk::Image &image, vk::Format format, vk::ImageSubresourceRange subResourceRange, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::ImageMemoryBarrier barrier(
    {},
    {},
        oldLayout,
        newLayout,
        {},
        {},
        image,
        subResourceRange
    );

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
    else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        sourceStage = vk::PipelineStageFlagBits::eBottomOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eColorAttachmentOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage,
        vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barriers);
}
void ResourceManager::SetImageLayoutInSingleCmd(vk::Image &image, vk::Format format, vk::ImageSubresourceRange subResourceRange, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::CommandBuffer commandBuffer = _beginSingleTimeCommand();

    SetImageLayout(commandBuffer, image, format, subResourceRange, oldLayout, newLayout);

    _endSingleTimeCommand(commandBuffer);
}

std::shared_ptr<VulkanTexture> ResourceManager::CreateVulkanTexture(std::shared_ptr<MyTexture> texture)
{
    std::shared_ptr<MyImage> myImage = texture->m_pImage;
    std::shared_ptr<VulkanTexture> vulkanTexture = std::make_shared<VulkanTexture>();

    _textures.push_back(vulkanTexture);

    vk::Format imageFormat = vk::Format::eR8G8B8A8Unorm;
    switch (myImage->m_format)
    {
    case MyImageFormat::MY_IMAGEFORMAT_RGBA8:
        imageFormat = vk::Format::eR8G8B8A8Unorm;
        break;
    case MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT:
        imageFormat = vk::Format::eR16G16B16A16Sfloat;
        break;
    }

    vk::ImageViewType imageViewType = vk::ImageViewType::e2D;
    vk::ImageCreateFlags flags = {};
    if (texture->m_pImage->m_layerCount == 6)
    {
        imageViewType = vk::ImageViewType::eCube;
        flags = vk::ImageCreateFlagBits::eCubeCompatible;
    }
    // create image

    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    if (texture->m_pImage->m_bFramebuffer)
    {
        usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
    }
    vk::ImageCreateInfo imageCreateInfo(flags,
        vk::ImageType::e2D,
        imageFormat,
        vk::Extent3D({
        static_cast<uint32_t>(myImage->m_width),
        static_cast<uint32_t>(myImage->m_height),
        1
    }),
        myImage->m_mipmapCount,
        myImage->m_layerCount,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        usage,
        vk::SharingMode::eExclusive,
        1,
        &_graphicsQueueFamilyIndex,
        vk::ImageLayout::eUndefined);
    VmaAllocationCreateInfo allocationCreateInfo = {};

    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateImage(_memoryAllocator, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo,
        reinterpret_cast<VkImage*>(&(vulkanTexture->image)), &(vulkanTexture->imageMemory), nullptr);

    vk::ImageSubresourceRange subResourceRange(
        vk::ImageAspectFlagBits::eColor,
        0,
        myImage->m_mipmapCount,
        0,
        myImage->m_layerCount
    );

    // create image view
    vk::ImageViewCreateInfo imageViewCreateInfo({
        {},
        vulkanTexture->image,
        imageViewType,
        imageFormat,
        vk::ComponentMapping({
        vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB,
        vk::ComponentSwizzle::eA
    }),
        subResourceRange
    });
    vulkanTexture->imageView = _device.createImageView(imageViewCreateInfo);

    return vulkanTexture;
}

// TODO: Refactor the following function and usage.
void ResourceManager::InitVulkanTextureData(std::shared_ptr<MyTexture> texture, std::shared_ptr<VulkanTexture> vulkanTexture)
{
    vk::Format imageFormat = vk::Format::eR8G8B8A8Unorm;
    switch (texture->m_pImage->m_format)
    {
    case MyImageFormat::MY_IMAGEFORMAT_RGBA8:
        imageFormat = vk::Format::eR8G8B8A8Unorm;
        break;
    case MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT:
        imageFormat = vk::Format::eR16G16B16A16Sfloat;
        break;
    }

    // transfer data from RAM to GPU memory
    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    stagingBufferInfo.size = texture->m_pImage->m_bufferSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo stagingBufferAllocInfo = {};
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaCreateBuffer(_memoryAllocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferMemory, nullptr);

    void* data;
    vmaMapMemory(_memoryAllocator, stagingBufferMemory, &data);
    memcpy(data, texture->m_pImage->m_data, static_cast<size_t>(stagingBufferInfo.size));
    vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

    vk::ImageSubresourceRange subResourceRange(
        vk::ImageAspectFlagBits::eColor,
        0,
        texture->m_pImage->m_mipmapCount,
        0,
        texture->m_pImage->m_layerCount
    );

    SetImageLayoutInSingleCmd(vulkanTexture->image, imageFormat, subResourceRange,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    _copyBufferToImage(stagingBuffer, vulkanTexture->image, texture->m_pImage);
    SetImageLayoutInSingleCmd(vulkanTexture->image, imageFormat, subResourceRange,
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);
}

vk::DescriptorSet ResourceManager::CreateTextureDescriptorSet(std::vector<std::shared_ptr<VulkanTexture>> textures)
{
    vk::DescriptorSet descriptorSet;
    std::vector<vk::DescriptorSetLayoutBinding> textureBindings;
    std::vector<vk::DescriptorImageInfo> imageInfos;

    for (size_t i = 0; i < textures.size(); ++i)
    {
        vk::DescriptorSetLayoutBinding textureBinding(i,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        textureBindings.push_back(textureBinding);

        vk::DescriptorImageInfo imageInfo(textures[i]->imageSampler,
            textures[i]->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.push_back(imageInfo);
    }

    vk::DescriptorSetLayout descriptorSetLayout;

    vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32_t>(textureBindings.size()), textureBindings.data());
    descriptorSetLayout = _device.createDescriptorSetLayout(layoutInfo);

    vk::DescriptorSetAllocateInfo allocInfo(_descriptorPool, 1, &descriptorSetLayout);

    _device.allocateDescriptorSets(&allocInfo, &descriptorSet);

    vk::WriteDescriptorSet descriptorWrite(descriptorSet,
        uint32_t(0),
        uint32_t(0),
        static_cast<uint32_t>(imageInfos.size()),
        vk::DescriptorType::eCombinedImageSampler,
        imageInfos.data(),
        {},
        {});
    _device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    _device.destroyDescriptorSetLayout(descriptorSetLayout);

    return descriptorSet;
}

std::shared_ptr<VulkanTexture> ResourceManager::CreateCombinedTexture(std::shared_ptr<MyTexture> texture)
{
    std::shared_ptr<VulkanTexture> vulkanTexture = CreateVulkanTexture(texture);

    vk::Format imageFormat = vk::Format::eR8G8B8A8Unorm;
    switch (texture->m_pImage->m_format)
    {
    case MyImageFormat::MY_IMAGEFORMAT_RGBA8:
        imageFormat = vk::Format::eR8G8B8A8Unorm;
        break;
    case MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT:
        imageFormat = vk::Format::eR16G16B16A16Sfloat;
        break;
    }

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
        texture->m_pImage->m_mipmapCount - 1,
        vk::BorderColor::eFloatOpaqueWhite,
        vk::Bool32(false)
    );

    vulkanTexture->imageSampler = _device.createSampler(createInfo);

    return vulkanTexture;
}

void ResourceManager::_createTextures(std::shared_ptr<Drawable> drawable)
{
    if (drawable->m_material->m_pDiffuseMap == nullptr && drawable->m_material->m_pNormalMap == nullptr)
    {
        return;
    }
    
    uint32_t bindings = 0;
    std::vector<vk::DescriptorSetLayoutBinding> textureBindings;
    std::vector<vk::DescriptorImageInfo> imageInfos;
    if (drawable->m_material->m_pDiffuseMap)
    {
        drawable->baseColorTexture = CreateCombinedTexture(drawable->m_material->m_pDiffuseMap);
        InitVulkanTextureData(drawable->m_material->m_pDiffuseMap, drawable->baseColorTexture);

        vk::DescriptorSetLayoutBinding textureBinding(bindings++,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        textureBindings.push_back(textureBinding);

        vk::DescriptorImageInfo imageInfo(drawable->baseColorTexture->imageSampler, 
            drawable->baseColorTexture->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.push_back(imageInfo);
    }
    if (drawable->m_material->m_pNormalMap)
    {
        drawable->normalTexture = CreateCombinedTexture(drawable->m_material->m_pNormalMap);
        InitVulkanTextureData(drawable->m_material->m_pNormalMap, drawable->normalTexture);
        vk::DescriptorSetLayoutBinding textureBinding(bindings++,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        textureBindings.push_back(textureBinding);

        vk::DescriptorImageInfo imageInfo(drawable->normalTexture->imageSampler,
            drawable->normalTexture->imageView,
            vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.push_back(imageInfo);
    }

    vk::DescriptorSetLayout descriptorSetLayout;

    vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32_t>(textureBindings.size()), textureBindings.data());
    descriptorSetLayout = _device.createDescriptorSetLayout(layoutInfo);

    vk::DescriptorSetAllocateInfo allocInfo(_descriptorPool, 1, &descriptorSetLayout);
    
    _device.allocateDescriptorSets(&allocInfo, &drawable->textureDescriptorSet);

    vk::WriteDescriptorSet descriptorWrite( drawable->textureDescriptorSet,
                                            uint32_t(0),
                                            uint32_t(0),
                                            static_cast<uint32_t>(imageInfos.size()),
                                            vk::DescriptorType::eCombinedImageSampler,
                                            imageInfos.data(),
                                            {},
                                            {} );
    _device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

    _device.destroyDescriptorSetLayout(descriptorSetLayout);
}