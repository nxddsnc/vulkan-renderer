#include "ResourceManager.h"
#include "Drawable.h"
#include "MyMesh.h"
#include "MyMaterial.h"

ResourceManager::ResourceManager(vk::Device &device, vk::CommandPool &commandPool, vk::Queue &graphicsQueue, uint32_t graphicsQueueFamilyIndex, VmaAllocator memoryAllocator)
{
    _device         = device;
    _commandPool    = commandPool;
    _graphicsQueue = graphicsQueue;
    _graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    _memoryAllocator = memoryAllocator;
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
    }
}

void ResourceManager::createNodeResource(std::shared_ptr<Drawable> drawable)
{
    _createVertexBuffers(drawable);
    _createIndexBuffer(drawable);
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