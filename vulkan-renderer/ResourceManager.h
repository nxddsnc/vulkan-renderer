#include "Platform.h"
#include "MyMesh.h"
#include <vector>
#include <memory.h>
#pragma once
/************************************************************************/
/* Meant to Control the gpu resource allocation.*/
/************************************************************************/
struct Drawable;
struct VulkanTexture;
class  MyTexture;
class ResourceManager
{
public:
    ResourceManager(vk::Device &device, vk::CommandPool &commandPool, vk::Queue &graphicsQueue,
        uint32_t graphicsQueueFamilyIndex, VmaAllocator memoryAllocator, vk::DescriptorPool &descriptorPool);
    ~ResourceManager();

    void createNodeResource(std::shared_ptr<Drawable> node);
private:
    void _createVertexBuffer(std::shared_ptr<Drawable> drawable, vk::DeviceSize size, void *data_);
    void _createVertexBuffers(std::shared_ptr<Drawable> drawable);
    void _createIndexBuffer(std::shared_ptr<Drawable> drawable);
    void _createTextures(std::shared_ptr<Drawable> drawable);
    void _transitionImageLayout(vk::Image &image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    std::shared_ptr<VulkanTexture> _createCombinedTexture(std::shared_ptr<MyTexture> texture);
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
    vk::DescriptorPool                            _descriptorPool;
    std::vector<std::shared_ptr<Drawable>>        _nodes;
};

