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
   /* for (auto node : _nodes)
    {
        vmaDestroyBuffer(_memoryAllocator, node->vertexBuffer, node->vertexBufferMemory);
        vmaDestroyBuffer(_memoryAllocator, node->indexBuffer, node->indexBufferMemory);
    }*/
}

void ResourceManager::createNodeResource(std::shared_ptr<Drawable> node)
{
    _createVertexBuffer(node);
    _createIndexBuffer(node);
    _nodes.push_back(node);
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


void ResourceManager::_createVertexBuffer(std::shared_ptr<Drawable> node)
{
    vk::DeviceSize size = sizeof(node->mesh->m_vertices[0]) * node->mesh->m_vertices.size();

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
    //vmaCreateBuffer(_memoryAllocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocInfo, reinterpret_cast<VkBuffer*>(&node->vertexBuffer), &node->vertexBufferMemory, nullptr);

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
    memcpy(data, node->mesh->m_vertices.data(), (size_t)size);
    vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

    //_copyBuffer(stagingBuffer, node->vertexBuffer, size);

    vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);
}

void ResourceManager::_createIndexBuffer(std::shared_ptr<Drawable> node)
{
    VkDeviceSize size = node->mesh->m_indexNum * node->mesh->getIndexSize();

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
    //vmaCreateBuffer(_memoryAllocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocInfo, reinterpret_cast<VkBuffer*>(&node->indexBuffer), &node->indexBufferMemory, nullptr);

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
    memcpy(data, node->mesh->m_indices, (size_t)size);
    vmaUnmapMemory(_memoryAllocator, stagingBufferMemory);

    //_copyBuffer(stagingBuffer, node->indexBuffer, size);

    vmaDestroyBuffer(_memoryAllocator, stagingBuffer, stagingBufferMemory);
}