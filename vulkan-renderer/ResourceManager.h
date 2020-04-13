#include "Platform.h"
#include "MyMesh.h"
#include <vector>
#include <memory.h>
#include "MyImage.h"
#include <unordered_map>
#pragma once
/************************************************************************/
/* Meant to Control the gpu resource allocation.*/
/************************************************************************/
class Drawable;
struct VulkanTexture;
class  MyTexture;
class ResourceManager
{
public:
    ResourceManager(vk::Device &device, vk::CommandPool &commandPool, vk::Queue &graphicsQueue,
        uint32_t graphicsQueueFamilyIndex, VmaAllocator memoryAllocator, vk::DescriptorPool &descriptorPool, vk::PhysicalDevice &gpu);
    ~ResourceManager();

    // TODO:: move the following functions to drawable class.
    void InitVulkanBuffers(std::shared_ptr<Drawable> drawable);
    void InitVulkanResource(std::shared_ptr<Drawable> drawable);

    std::shared_ptr<VulkanTexture> CreateVulkanTexture(std::shared_ptr<MyTexture> texture);
    std::shared_ptr<VulkanTexture> CreateCombinedTexture(std::shared_ptr<MyTexture> texture);
	std::shared_ptr<VulkanTexture> GetVulkanTexture(std::shared_ptr<MyTexture> texture);

    void CreateVertexBuffers(std::shared_ptr<Drawable> drawable);
    void CreateIndexBuffer(std::shared_ptr<MyMesh> mesh, vk::Buffer &buffer, VmaAllocation &bufferMemory);
    void InitVertexBuffer(vk::DeviceSize size, void *data_, vk::Buffer &buffer, VmaAllocation &bufferMemory, vk::DeviceSize &bufferOffset);
    void SetImageLayout(vk::CommandBuffer &command, vk::Image &image, vk::Format format, vk::ImageSubresourceRange subResourceRange, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void SetImageLayoutInSingleCmd(vk::Image &image, vk::Format format, vk::ImageSubresourceRange subResourceRange, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void InitVulkanTextureData(std::shared_ptr<MyTexture> texture, std::shared_ptr<VulkanTexture> vulkanTexture);
    vk::DescriptorSet CreateTextureDescriptorSet(std::vector<std::shared_ptr<VulkanTexture>> textures);
	void CreateUniformBuffer(size_t size, VkBuffer* buffer, VmaAllocation* bufferMemory);
	void DestroyUniformBuffer(vk::Buffer &buffer, VmaAllocation &bufferMemory);
	void UpdateBuffer(VmaAllocation bufferMemory, char* src, int size);
	void TransferGPUTextureToCPU(std::shared_ptr<VulkanTexture> src, std::shared_ptr<MyTexture> dst);
private:
    void _createTextures(std::shared_ptr<Drawable> drawable);
    vk::CommandBuffer _beginSingleTimeCommand();
    void _endSingleTimeCommand(vk::CommandBuffer &commandBuffer);
    void _copyBufferToImage(vk::Buffer buffer, vk::Image image, std::shared_ptr<MyImage> myImage);
    void _copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
public:
	vk::PhysicalDevice												 m_gpu;
	vk::Device														 m_device;
	vk::Queue														 m_graphicsQueue;
	vk::CommandPool													 m_commandPool;
	uint32_t														 m_graphicsQueueFamilyIndex;
	VmaAllocator													 m_memoryAllocator;
	vk::DescriptorPool												 m_descriptorPool;

private:
	std::vector<std::shared_ptr<Drawable>>							 m_drawables;
	std::unordered_map<std::string, std::shared_ptr<VulkanTexture>>  m_textureMap;
};

