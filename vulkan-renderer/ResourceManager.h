//#include "Platform.h"
//#pragma once
//class ResourceManager
//{
//public:
//	ResourceManager(VkDevice *device, VkCommandPool *commandPool, );
//	~ResourceManager();
//
//private:
//	VkDevice					* _device;
//	VkQueue						* _graphicsQueue;
//	VkCommandPool				* _commandPool;
//
//
//	//uint32_t findMemoryTypeIndex(VkPhysicalDeviceMemoryProperties * gpuMemoryProperties,
//	//	const VkMemoryRequirements * memoryRequiremenets, const VkMemoryPropertyFlags requiredProperties)
//	//{
//	//	for (uint32_t i = 0; i < gpuMemoryProperties->memoryTypeCount; ++i)
//	//	{
//	//		if (memoryRequiremenets->memoryTypeBits & (1 << i))
//	//		{
//	//			if ((gpuMemoryProperties->memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties)
//	//			{
//	//				return i;
//	//			}
//	//		}
//	//	}
//	//	assert(1 && "could not find memory type index");
//	//	return UINT32_MAX;
//	//}
//
//	//void createBuffer(VkDevice device, VkDeviceSize size,
//	//	VkPhysicalDeviceMemoryProperties *gpuMemoryProperties,
//	//	VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
//	//	VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
//
//	//	VkBufferCreateInfo bufferInfo = {};
//	//	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	//	bufferInfo.size = size;
//	//	bufferInfo.usage = usage;
//	//	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//	//	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
//	//		throw std::runtime_error("failed to create buffer!");
//	//	}
//
//	//	VkMemoryRequirements memRequirements;
//	//	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
//
//	//	VkMemoryAllocateInfo allocInfo = {};
//	//	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	//	allocInfo.allocationSize = memRequirements.size;
//	//	allocInfo.memoryTypeIndex = FindMemoryTypeIndex(gpuMemoryProperties, &memRequirements, properties);
//
//	//	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
//	//		throw std::runtime_error("failed to allocate buffer memory!");
//	//	}
//
//	//	vkBindBufferMemory(device, buffer, bufferMemory, 0);
//	//}
//
//	//void copyBuffer(VkDevice &device, VkQueue &graphicsQueue, VkCommandPool &commandPool, VkBuffer srcBuffer,
//	//	VkBuffer dstBuffer, VkDeviceSize size)
//	//{
//	//	VkCommandBufferAllocateInfo allocInfo = {};
//	//	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	//	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//	//	allocInfo.commandPool = commandPool;
//	//	allocInfo.commandBufferCount = 1;
//
//	//	VkCommandBuffer commandBuffer;
//	//	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
//
//	//	VkCommandBufferBeginInfo beginInfo = {};
//	//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//	//	vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//	//	VkBufferCopy copyRegion = {};
//	//	copyRegion.srcOffset = 0; // Optional
//	//	copyRegion.dstOffset = 0; // Optional
//	//	copyRegion.size = size;
//	//	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
//
//	//	vkEndCommandBuffer(commandBuffer);
//
//	//	VkSubmitInfo submitInfo = {};
//	//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	//	submitInfo.commandBufferCount = 1;
//	//	submitInfo.pCommandBuffers = &commandBuffer;
//
//	//	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
//	//	vkQueueWaitIdle(graphicsQueue);
//
//	//	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
//	//}
//
//	//void createImage(VkDevice &device, VkPhysicalDeviceMemoryProperties * gpuMemoryProperties,
//	//	uint32_t width, uint32_t height, VkFormat format,
//	//	VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
//	//	VkImage& image, VkDeviceMemory& imageMemory) {
//	//	VkImageCreateInfo imageInfo = {};
//	//	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//	//	imageInfo.imageType = VK_IMAGE_TYPE_2D;
//	//	imageInfo.extent.width = width;
//	//	imageInfo.extent.height = height;
//	//	imageInfo.extent.depth = 1;
//	//	imageInfo.mipLevels = 1;
//	//	imageInfo.arrayLayers = 1;
//	//	imageInfo.format = format;
//	//	imageInfo.tiling = tiling;
//	//	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	//	imageInfo.usage = usage;
//	//	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//	//	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//	//	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
//	//		throw std::runtime_error("failed to create image!");
//	//	}
//
//	//	VkMemoryRequirements memRequirements;
//	//	vkGetImageMemoryRequirements(device, image, &memRequirements);
//
//	//	VkMemoryAllocateInfo allocInfo = {};
//	//	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	//	allocInfo.allocationSize = memRequirements.size;
//	//	allocInfo.memoryTypeIndex = FindMemoryTypeIndex(gpuMemoryProperties, &memRequirements, properties);
//
//	//	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
//	//		throw std::runtime_error("failed to allocate image memory!");
//	//	}
//
//	//	vkBindImageMemory(device, image, imageMemory, 0);
//	//}
//
//	//VkCommandBuffer beginSingleTimeCommands(VkDevice &device, VkCommandPool &commandPool) {
//	//	VkCommandBufferAllocateInfo allocInfo = {};
//	//	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	//	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//	//	allocInfo.commandPool = commandPool;
//	//	allocInfo.commandBufferCount = 1;
//
//	//	VkCommandBuffer commandBuffer;
//	//	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
//
//	//	VkCommandBufferBeginInfo beginInfo = {};
//	//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//	//	vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//	//	return commandBuffer;
//	//}
//
//	//void endSingleTimeCommands(VkDevice &device, VkCommandPool &commandPool, VkQueue &graphicsQueue, VkCommandBuffer commandBuffer) {
//	//	vkEndCommandBuffer(commandBuffer);
//
//	//	VkSubmitInfo submitInfo = {};
//	//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	//	submitInfo.commandBufferCount = 1;
//	//	submitInfo.pCommandBuffers = &commandBuffer;
//
//	//	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
//	//	vkQueueWaitIdle(graphicsQueue);
//
//	//	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
//	//}
//
//	//void copyBuffer(VkDevice &device, VkCommandPool &commandPool, VkQueue &graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
//	//	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
//
//	//	VkBufferCopy copyRegion = {};
//	//	copyRegion.size = size;
//	//	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
//
//	//	endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
//	//}
//
//	//void transitionImageLayout(VkDevice &device, VkCommandPool &commandPool, VkQueue &graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
//	//	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
//
//	//	VkImageMemoryBarrier barrier = {};
//	//	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//	//	barrier.oldLayout = oldLayout;
//	//	barrier.newLayout = newLayout;
//
//	//	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//	//	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//
//	//	barrier.image = image;
//	//	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//	//	barrier.subresourceRange.baseMipLevel = 0;
//	//	barrier.subresourceRange.levelCount = 1;
//	//	barrier.subresourceRange.baseArrayLayer = 0;
//	//	barrier.subresourceRange.layerCount = 1;
//
//	//	barrier.srcAccessMask = 0; // TODO
//	//	barrier.dstAccessMask = 0; // TODO
//
//	//	vkCmdPipelineBarrier(
//	//		commandBuffer,
//	//		0 /* TODO */, 0 /* TODO */,
//	//		0,
//	//		0, nullptr,
//	//		0, nullptr,
//	//		1, &barrier
//	//	);
//
//	//	endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
//	//}
//
//	//void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
//	//	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
//
//	//	VkBufferImageCopy region = {};
//	//	region.bufferOffset = 0;
//	//	region.bufferRowLength = 0;
//	//	region.bufferImageHeight = 0;
//
//	//	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//	//	region.imageSubresource.mipLevel = 0;
//	//	region.imageSubresource.baseArrayLayer = 0;
//	//	region.imageSubresource.layerCount = 1;
//
//	//	region.imageOffset = { 0, 0, 0 };
//	//	region.imageExtent = {
//	//		width,
//	//		height,
//	//		1
//	//	};
//
//	//	endSingleTimeCommands(commandBuffer);
//	//}
//
//};
//
