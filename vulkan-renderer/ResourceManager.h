#include "Platform.h"
#include "MyMesh.h"
#include <vector>
#include <memory.h>
#pragma once
/************************************************************************/
/* Meant to Control the gpu resource allocation.*/
/************************************************************************/
struct Drawable;
class ResourceManager
{
public:
    ResourceManager(vk::Device &device, vk::CommandPool &commandPool, vk::Queue &graphicsQueue, uint32_t graphicsQueueFamilyIndex, VmaAllocator memoryAllocator);
    ~ResourceManager();

    void createNodeResource(std::shared_ptr<Drawable> node);
private:
    void _createVertexBuffer(std::shared_ptr<Drawable> drawable, vk::DeviceSize size, void *data_);
    void _createVertexBuffers(std::shared_ptr<Drawable> node);
    void _createIndexBuffer(std::shared_ptr<Drawable> node);
    vk::CommandBuffer _beginSingleTimeCommand();
    void _endSingleTimeCommand(vk::CommandBuffer &commandBuffer);
    void _copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
    void _copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
private:
    vk::Device                                    _device;
    vk::Queue                                     _graphicsQueue;
    vk::CommandPool                               _commandPool;
    uint32_t                                      _graphicsQueueFamilyIndex;
    VmaAllocator                                  _memoryAllocator;
    std::vector<std::shared_ptr<Drawable>>        _nodes;
};

