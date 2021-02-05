#include "ResourceManager.h"
#include "Renderable.h"
#include "MyMesh.h"
#include "MyMaterial.h"
#include "MyTexture.h"
#include "MyImage.h"
#include <algorithm>

ResourceManager::ResourceManager(vk::Device &device, vk::CommandPool &commandPool, vk::Queue &graphicsQueue,
    uint32_t graphicsQueueFamilyIndex, VmaAllocator memoryAllocator, vk::PhysicalDevice &gpu)
{
    m_device         = device;
    m_commandPool    = commandPool;
    m_graphicsQueue = graphicsQueue;
    m_graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    m_memoryAllocator = memoryAllocator;
    m_gpu = gpu;
}

ResourceManager::~ResourceManager()
{
    for (auto renderable : m_renderables)
    {
        for (int i = 0; i < renderable->m_vertexBuffers.size(); ++i)
        {
            vmaDestroyBuffer(m_memoryAllocator, renderable->m_vertexBuffers[i], renderable->m_vertexBufferMemorys[i]);
        }
        vmaDestroyBuffer(m_memoryAllocator, renderable->m_indexBuffer, renderable->m_indexBufferMemory);

		if (renderable->m_type == INSTANCE_RENDERABLE)
		{
			auto renderable_ = std::dynamic_pointer_cast<InstanceRenderable>(renderable);
			for (int i = 0; i < renderable_->m_instanceBuffer.size(); ++i)
			{
				vmaDestroyBuffer(m_memoryAllocator, renderable_->m_instanceBuffer[i], renderable_->m_instanceBufferMemory[i]);
			}
		}

    }



    for (auto pair : m_textureMap)
    {
        vmaDestroyImage(m_memoryAllocator, pair.second->image, pair.second->imageMemory);
        if (pair.second->imageView)
        {
            m_device.destroyImageView(pair.second->imageView);
        }
        if (pair.second->imageSampler)
        {
            m_device.destroySampler(pair.second->imageSampler);
        }
    }
}

void ResourceManager::InitVulkanBuffers(std::shared_ptr<Renderable> renderable)
{
    CreateVertexBuffers(renderable);
    CreateIndexBuffer(renderable->m_mesh, renderable->m_indexBuffer, renderable->m_indexBufferMemory);
    m_renderables.push_back(renderable);
}

void ResourceManager::InitVulkanResource(std::shared_ptr<Renderable> renderable)
{
	if (renderable->m_bReady)
	{
		return;
	}
    InitVulkanBuffers(renderable);
    _createTextures(renderable);
	renderable->m_bReady = true;
}

vk::CommandBuffer ResourceManager::BeginSingleTimeCommand()
{
    vk::CommandBufferAllocateInfo allocInfo({
        m_commandPool,
        vk::CommandBufferLevel::ePrimary,
        1
    });
    auto commandBuffers = m_device.allocateCommandBuffers(allocInfo);

    vk::CommandBufferBeginInfo beginInfo({
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
        {}
    });
    commandBuffers[0].begin(&beginInfo);
    return commandBuffers[0];
}

void ResourceManager::EndSingleTimeCommand(vk::CommandBuffer &commandBuffer)
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
    m_graphicsQueue.submit((uint32_t)1, &submitInfo, nullptr);
    m_graphicsQueue.waitIdle();
    m_device.freeCommandBuffers(m_commandPool, 1, &commandBuffer);
}

void ResourceManager::_copyBufferToImage(vk::Buffer buffer, vk::Image image, std::shared_ptr<MyImage> myImage)
{
    vk::CommandBuffer cmdBuffer = BeginSingleTimeCommand();

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

    EndSingleTimeCommand(cmdBuffer);
}

void ResourceManager::_copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommand();

    vk::BufferCopy copyRegion(
        0,
        0,
        size
    );
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

    EndSingleTimeCommand(commandBuffer);
}

void ResourceManager::InitVertexBuffer(vk::DeviceSize size, void *data_, vk::Buffer &buffer, VmaAllocation &bufferMemory, vk::DeviceSize &bufferOffset, vk::BufferUsageFlagBits usage)
{
    vk::BufferCreateInfo bufferInfo({
        {},
        size,
		usage | vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        1,
        &m_graphicsQueueFamilyIndex
    });

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateBuffer(m_memoryAllocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocInfo, reinterpret_cast<VkBuffer*>(&buffer), &bufferMemory, nullptr);

    vk::Buffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    vk::BufferCreateInfo stagingBufferInfo({
        {},
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        1,
        &m_graphicsQueueFamilyIndex
    });
    VmaAllocationCreateInfo stagingBufferAllocInfo = {};
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaCreateBuffer(m_memoryAllocator, reinterpret_cast<VkBufferCreateInfo*>(&stagingBufferInfo), &stagingBufferAllocInfo, reinterpret_cast<VkBuffer*>(&stagingBuffer), &stagingBufferMemory, nullptr);

    void* data;
    vmaMapMemory(m_memoryAllocator, stagingBufferMemory, &data);
    memcpy(data, data_, (size_t)size);
    vmaUnmapMemory(m_memoryAllocator, stagingBufferMemory);

    _copyBuffer(stagingBuffer, buffer, size);

    vmaDestroyBuffer(m_memoryAllocator, stagingBuffer, stagingBufferMemory);

    bufferOffset = 0;
}

void ResourceManager::CreateVertexBuffers(std::shared_ptr<Renderable> renderable)
{
    vk::DeviceSize size = sizeof(renderable->m_mesh->m_positions[0]) * renderable->m_mesh->m_positions.size();
    vk::Buffer positionBuffer;
    VmaAllocation positionBufferMemory;
    vk::DeviceSize positionBufferOffset;
    InitVertexBuffer(size, reinterpret_cast<void*>(renderable->m_mesh->m_positions.data()), positionBuffer, positionBufferMemory, positionBufferOffset);
    renderable->m_vertexBuffers.push_back(std::move(positionBuffer));
    renderable->m_vertexBufferMemorys.push_back(std::move(positionBufferMemory));
    renderable->m_vertexBufferOffsets.push_back(std::move(positionBufferOffset));

    if (renderable->m_mesh->m_normals.size() > 0)
    {
        vk::Buffer normalBuffer;
        VmaAllocation normalBufferMemory;
        vk::DeviceSize normalBufferOffset;
        size = sizeof(renderable->m_mesh->m_normals[0]) * renderable->m_mesh->m_normals.size();
        InitVertexBuffer(size, reinterpret_cast<void*>(renderable->m_mesh->m_normals.data()), normalBuffer, normalBufferMemory, normalBufferOffset);
        renderable->m_vertexBuffers.push_back(std::move(normalBuffer));
        renderable->m_vertexBufferMemorys.push_back(std::move(normalBufferMemory));
        renderable->m_vertexBufferOffsets.push_back(std::move(normalBufferOffset));
    }

    if (renderable->m_mesh->m_texCoords0.size() > 0)
    {
        vk::Buffer uvBuffer;
        VmaAllocation uvBufferMemory;
        vk::DeviceSize uvBufferOffset;
        size = sizeof(renderable->m_mesh->m_texCoords0[0]) * renderable->m_mesh->m_texCoords0.size();
        InitVertexBuffer(size, reinterpret_cast<void*>(renderable->m_mesh->m_texCoords0.data()), uvBuffer, uvBufferMemory, uvBufferOffset);
        renderable->m_vertexBuffers.push_back(std::move(uvBuffer));
        renderable->m_vertexBufferMemorys.push_back(std::move(uvBufferMemory));
        renderable->m_vertexBufferOffsets.push_back(std::move(uvBufferOffset));
    }
    if (renderable->m_mesh->m_tangents.size() > 0)
    {
        vk::Buffer tangentBuffer;
        VmaAllocation tangentBufferMemory;
        vk::DeviceSize tangentBufferOffset;
        size = sizeof(renderable->m_mesh->m_tangents[0]) * renderable->m_mesh->m_tangents.size();
        InitVertexBuffer(size, reinterpret_cast<void*>(renderable->m_mesh->m_tangents.data()), tangentBuffer, tangentBufferMemory, tangentBufferOffset);
        renderable->m_vertexBuffers.push_back(std::move(tangentBuffer));
        renderable->m_vertexBufferMemorys.push_back(std::move(tangentBufferMemory));
        renderable->m_vertexBufferOffsets.push_back(std::move(tangentBufferOffset));
    }
    if (renderable->m_mesh->m_colors.size() > 0)
    {
        vk::Buffer colorBuffer;
        VmaAllocation colorBufferMemory;
        vk::DeviceSize colorBufferOffset;
        size = sizeof(renderable->m_mesh->m_colors[0]) * renderable->m_mesh->m_colors.size();
        InitVertexBuffer(size, reinterpret_cast<void*>(renderable->m_mesh->m_colors.data()), colorBuffer, colorBufferMemory, colorBufferOffset);
        renderable->m_vertexBuffers.push_back(std::move(colorBuffer));
        renderable->m_vertexBufferMemorys.push_back(std::move(colorBufferMemory));
        renderable->m_vertexBufferOffsets.push_back(std::move(colorBufferOffset));
    }
	if (renderable->m_mesh->m_joints.size() > 0)
	{
		vk::Buffer jointsBuffer;
		VmaAllocation jointsBufferMemory;
		vk::DeviceSize jointsBufferOffset;
		size = sizeof(renderable->m_mesh->m_joints[0]) * renderable->m_mesh->m_joints.size();
		InitVertexBuffer(size, reinterpret_cast<void*>(renderable->m_mesh->m_joints.data()), jointsBuffer, jointsBufferMemory, jointsBufferOffset);
		renderable->m_vertexBuffers.push_back(std::move(jointsBuffer));
		renderable->m_vertexBufferMemorys.push_back(std::move(jointsBufferMemory));
		renderable->m_vertexBufferOffsets.push_back(std::move(jointsBufferOffset));
	}
	if (renderable->m_mesh->m_weights.size() > 0)
	{
		vk::Buffer weightsBuffer;
		VmaAllocation weightsBufferMemory;
		vk::DeviceSize weightsBufferOffset;
		size = sizeof(renderable->m_mesh->m_weights[0]) * renderable->m_mesh->m_weights.size();
		InitVertexBuffer(size, reinterpret_cast<void*>(renderable->m_mesh->m_weights.data()), weightsBuffer, weightsBufferMemory, weightsBufferOffset);
		renderable->m_vertexBuffers.push_back(std::move(weightsBuffer));
		renderable->m_vertexBufferMemorys.push_back(std::move(weightsBufferMemory));
		renderable->m_vertexBufferOffsets.push_back(std::move(weightsBufferOffset));
	}
	if (renderable->m_type == INSTANCE_RENDERABLE)
	{
		std::shared_ptr<InstanceRenderable> instance = std::dynamic_pointer_cast<InstanceRenderable>(renderable);

		for (int i = 0; i < 3; ++i)
		{
			vk::Buffer instanceBuffer;
			VmaAllocation instanceBufferMemory;
			vk::DeviceSize instanceBufferOffset;
			size = sizeof(float) * 4 * instance->m_matricies.size();
			InitVertexBuffer(size, reinterpret_cast<void*>(instance->m_matrixCols[i].data()), instanceBuffer, instanceBufferMemory, instanceBufferOffset);
			instance->m_instanceBuffer.push_back(std::move(instanceBuffer));
			instance->m_instanceBufferMemory.push_back(std::move(instanceBufferMemory));
			instance->m_instanceBufferOffsets.push_back(instanceBufferOffset);
		}
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
        &m_graphicsQueueFamilyIndex
    });
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateBuffer(m_memoryAllocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocInfo, reinterpret_cast<VkBuffer*>(&buffer), &bufferMemory, nullptr);

    vk::Buffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    vk::BufferCreateInfo stagingBufferInfo({
        {},
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        1,
        &m_graphicsQueueFamilyIndex
    });
    VmaAllocationCreateInfo stagingBufferAllocInfo = {};
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaCreateBuffer(m_memoryAllocator, reinterpret_cast<VkBufferCreateInfo*>(&stagingBufferInfo), &stagingBufferAllocInfo, reinterpret_cast<VkBuffer*>(&stagingBuffer), &stagingBufferMemory, nullptr);

    void* data;
    vmaMapMemory(m_memoryAllocator, stagingBufferMemory, &data);
    memcpy(data, mesh->m_indices, (size_t)size);
    vmaUnmapMemory(m_memoryAllocator, stagingBufferMemory);

    _copyBuffer(stagingBuffer, buffer, size);

    vmaDestroyBuffer(m_memoryAllocator, stagingBuffer, stagingBufferMemory);
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
	else if (oldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		sourceStage = vk::PipelineStageFlagBits::eBottomOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	else if (oldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eBottomOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;

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
	else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eColorAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		sourceStage = vk::PipelineStageFlagBits::eFragmentShader;
		destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	else if (oldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		sourceStage = vk::PipelineStageFlagBits::eFragmentShader;
		destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		sourceStage = vk::PipelineStageFlagBits::eBottomOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eTopOfPipe;
	}
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage,
        vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barriers);
}
void ResourceManager::SetImageLayoutInSingleCmd(vk::Image &image, vk::Format format, vk::ImageSubresourceRange subResourceRange, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommand();

    SetImageLayout(commandBuffer, image, format, subResourceRange, oldLayout, newLayout);

    EndSingleTimeCommand(commandBuffer);
}

std::shared_ptr<VulkanTexture> ResourceManager::CreateVulkanTexture(std::shared_ptr<MyTexture> texture)
{
	std::shared_ptr<MyImage> myImage = texture->m_pImage;

	std::shared_ptr<VulkanTexture> vulkanTexture;

	char key[512];

	sprintf(key, "%s-%d-%d", texture->m_pImage->m_fileName, texture->m_pImage->m_width, texture->m_pImage->m_height);

	if (m_textureMap.count(key) > 0)
	{
		vulkanTexture = m_textureMap.at(key);
		vulkanTexture->referenceCount++;
		return vulkanTexture;
	}
	else
	{
		vulkanTexture = std::make_shared<VulkanTexture>();
		vulkanTexture->referenceCount = 1;
		m_textureMap.insert(std::make_pair(key, vulkanTexture));
	}

	vulkanTexture->texture = texture;

	bool bDepthTexture = false;
	vulkanTexture->format = vk::Format::eR8G8B8A8Unorm;
	switch (myImage->m_format)
	{
	case MyImageFormat::MY_IMAGEFORMAT_RGBA8:
		vulkanTexture->format = vk::Format::eR8G8B8A8Unorm;
		break;
	case MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT:
		vulkanTexture->format = vk::Format::eR16G16B16A16Sfloat;
		break;
	case MyImageFormat::MY_IMAGEFORMAT_R16_FLOAT:
		vulkanTexture->format = vk::Format::eR16Sfloat;
		break;
	case MyImageFormat::MY_IMAGEFORMAT_R32_FLOAT:
		vulkanTexture->format = vk::Format::eR32Sfloat;
		break;
	case MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT:
		vulkanTexture->format = vk::Format::eD24UnormS8Uint;
		bDepthTexture = true;
		break;
	case MyImageFormat::MY_IMAGEFORMAT_D32_FLOAT:
		vulkanTexture->format = vk::Format::eD32Sfloat;
		bDepthTexture = true;
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
	if (texture->m_pImage->m_bTransferSrc)
	{
		usage |= vk::ImageUsageFlagBits::eTransferSrc;
	}
	if (texture->m_pImage->m_bFramebuffer)
    {
		if (bDepthTexture)
		{
			// FIXME: Too many usage being set may cause performance issue.
			usage = vk::ImageUsageFlagBits::eDepthStencilAttachment| vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		}
		else
		{
			usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
		}
    }
    vk::ImageCreateInfo imageCreateInfo(flags,
        vk::ImageType::e2D,
        vulkanTexture->format,
        vk::Extent3D(
            static_cast<uint32_t>(myImage->m_width),
            static_cast<uint32_t>(myImage->m_height),
        1
        ),
        myImage->m_mipmapCount,
        myImage->m_layerCount,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        usage,
        vk::SharingMode::eExclusive,
        1,
        &m_graphicsQueueFamilyIndex,
        vk::ImageLayout::eUndefined);
    VmaAllocationCreateInfo allocationCreateInfo = {};

    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (texture->m_pImage->m_bHostVisible)
	{
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		imageCreateInfo.tiling = vk::ImageTiling::eLinear;
	}
    vmaCreateImage(m_memoryAllocator, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo,
        reinterpret_cast<VkImage*>(&(vulkanTexture->image)), &(vulkanTexture->imageMemory), nullptr);

    vk::ImageSubresourceRange subResourceRange(
		bDepthTexture? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor,
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
        vulkanTexture->format,
        vk::ComponentMapping({
        vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB,
        vk::ComponentSwizzle::eA
    }),
        subResourceRange
    });
    vulkanTexture->imageView = m_device.createImageView(imageViewCreateInfo);

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
	case MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT:
		imageFormat = vk::Format::eD24UnormS8Uint;
		break;
	case MyImageFormat::MY_IMAGEFORMAT_D32_FLOAT:
		imageFormat = vk::Format::eD32Sfloat;
		break;
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
    vmaCreateBuffer(m_memoryAllocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferMemory, nullptr);

    void* data;
    vmaMapMemory(m_memoryAllocator, stagingBufferMemory, &data);
    memcpy(data, texture->m_pImage->m_data, static_cast<size_t>(stagingBufferInfo.size));
    vmaUnmapMemory(m_memoryAllocator, stagingBufferMemory);

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
    vmaDestroyBuffer(m_memoryAllocator, stagingBuffer, stagingBufferMemory);
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
    descriptorSetLayout = m_device.createDescriptorSetLayout(layoutInfo);

    vk::DescriptorSetAllocateInfo allocInfo(m_descriptorPool, 1, &descriptorSetLayout);

    m_device.allocateDescriptorSets(&allocInfo, &descriptorSet);

    vk::WriteDescriptorSet descriptorWrite(descriptorSet,
        uint32_t(0),
        uint32_t(0),
        static_cast<uint32_t>(imageInfos.size()),
        vk::DescriptorType::eCombinedImageSampler,
        imageInfos.data(),
        {},
        {});
    m_device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    m_device.destroyDescriptorSetLayout(descriptorSetLayout);

    return descriptorSet;
}

void ResourceManager::CreateUniformBuffer(size_t size, VkBuffer* buffer, VmaAllocation* bufferMemory)
{
	vk::BufferCreateInfo createInfo({ {},
		vk::DeviceSize(size),
		vk::BufferUsageFlagBits::eUniformBuffer,
		{},
		{},
		{} });
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	VkBufferCreateInfo &vkCreateInfo = createInfo;
	vmaCreateBuffer(m_memoryAllocator, &vkCreateInfo, &allocInfo, buffer, bufferMemory, nullptr);
}

void ResourceManager::DestroyUniformBuffer(vk::Buffer &buffer, VmaAllocation &bufferMemory)
{
	vmaDestroyBuffer(m_memoryAllocator, buffer, bufferMemory);
}

void ResourceManager::UpdateBuffer(VmaAllocation bufferMemory, char* src, int size)
{
	char* data = nullptr;
	vmaMapMemory(m_memoryAllocator, bufferMemory, reinterpret_cast<void**>(&data));
	memcpy(data, src, size);
	vmaUnmapMemory(m_memoryAllocator, bufferMemory);
}

void ResourceManager::TransferGPUTextureToCPU(std::shared_ptr<VulkanTexture> src, std::shared_ptr<MyTexture> dst)
{
	void *data;
	vmaMapMemory(m_memoryAllocator, src->imageMemory, &data);
	
	if (dst->m_pImage->m_data == nullptr)
	{
		dst->m_pImage->m_data = new char[dst->m_pImage->m_bufferSize];
	}
	memcpy(dst->m_pImage->m_data, data, static_cast<size_t>(dst->m_pImage->m_bufferSize));
	vmaUnmapMemory(m_memoryAllocator, src->imageMemory);
}

std::shared_ptr<VulkanTexture> ResourceManager::CreateCombinedTexture(std::shared_ptr<MyTexture> texture)
{
    std::shared_ptr<VulkanTexture> vulkanTexture = CreateVulkanTexture(texture);
	
	if (vulkanTexture->imageSampler)
	{
		return vulkanTexture;
	}
    vk::Format imageFormat = vk::Format::eR8G8B8A8Unorm;
	bool bDepthTexture = false;
	switch (texture->m_pImage->m_format)
    {
    case MyImageFormat::MY_IMAGEFORMAT_RGBA8:
        imageFormat = vk::Format::eR8G8B8A8Unorm;
        break;
    case MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT:
        imageFormat = vk::Format::eR16G16B16A16Sfloat;
        break;
	case MyImageFormat::MY_IMAGEFORMAT_D32_FLOAT:
		imageFormat = vk::Format::eD32Sfloat;
		bDepthTexture = true;
		break;
    }

	vk::CompareOp compareOp = vk::CompareOp::eAlways;
	if (bDepthTexture)
	{
		compareOp = vk::CompareOp::eGreaterOrEqual;
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
        vk::Bool32(bDepthTexture),
        compareOp,
        0.0,
        texture->m_pImage->m_mipmapCount - 1,
        vk::BorderColor::eFloatOpaqueWhite,
        vk::Bool32(false)
    );

    vulkanTexture->imageSampler = m_device.createSampler(createInfo);

    return vulkanTexture;
}

std::shared_ptr<VulkanTexture> ResourceManager::GetVulkanTexture(std::shared_ptr<MyTexture> texture)
{
	char key[512];
	sprintf(key, "%s-%d-%d", texture->m_pImage->m_fileName, texture->m_pImage->m_width, texture->m_pImage->m_height);

	if (m_textureMap.count(key) > 0)
	{
		return m_textureMap.at(key);
	}
	else
	{
		return nullptr;
	}
}

void ResourceManager::_createTextures(std::shared_ptr<Renderable> renderable)
{
    if (renderable->m_material->m_pDiffuseMap == nullptr && renderable->m_material->m_pNormalMap == nullptr)
    {
        return;
    }
    
    uint32_t bindings = 0;
    std::vector<vk::DescriptorSetLayoutBinding> textureBindings;
    std::vector<vk::DescriptorImageInfo> imageInfos;
    if (renderable->m_material->m_pDiffuseMap)
    {
        renderable->baseColorTexture = CreateCombinedTexture(renderable->m_material->m_pDiffuseMap);
        InitVulkanTextureData(renderable->m_material->m_pDiffuseMap, renderable->baseColorTexture);

        vk::DescriptorSetLayoutBinding textureBinding(bindings++,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        textureBindings.push_back(textureBinding);

        vk::DescriptorImageInfo imageInfo(renderable->baseColorTexture->imageSampler, 
            renderable->baseColorTexture->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.push_back(imageInfo);
    }
    if (renderable->m_material->m_pNormalMap)
    {
        renderable->normalTexture = CreateCombinedTexture(renderable->m_material->m_pNormalMap);
        InitVulkanTextureData(renderable->m_material->m_pNormalMap, renderable->normalTexture);
        vk::DescriptorSetLayoutBinding textureBinding(bindings++,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        textureBindings.push_back(textureBinding);

        vk::DescriptorImageInfo imageInfo(renderable->normalTexture->imageSampler,
            renderable->normalTexture->imageView,
            vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.push_back(imageInfo);
    }
    if (renderable->m_material->m_pMetallicRoughnessMap)
    {
        renderable->metallicRoughnessTexture = CreateCombinedTexture(renderable->m_material->m_pMetallicRoughnessMap);
        InitVulkanTextureData(renderable->m_material->m_pMetallicRoughnessMap, renderable->metallicRoughnessTexture);
        vk::DescriptorSetLayoutBinding textureBinding(bindings++,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        textureBindings.push_back(textureBinding);

        vk::DescriptorImageInfo imageInfo(renderable->metallicRoughnessTexture->imageSampler,
            renderable->metallicRoughnessTexture->imageView,
            vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.push_back(imageInfo);
    }

    vk::DescriptorSetLayout descriptorSetLayout;

    vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32_t>(textureBindings.size()), textureBindings.data());
    descriptorSetLayout = m_device.createDescriptorSetLayout(layoutInfo);

    vk::DescriptorSetAllocateInfo allocInfo(m_descriptorPool, 1, &descriptorSetLayout);
    
    m_device.allocateDescriptorSets(&allocInfo, &renderable->textureDescriptorSet);

    vk::WriteDescriptorSet descriptorWrite( renderable->textureDescriptorSet,
                                            uint32_t(0),
                                            uint32_t(0),
                                            static_cast<uint32_t>(imageInfos.size()),
                                            vk::DescriptorType::eCombinedImageSampler,
                                            imageInfos.data(),
                                            {},
                                            {} );
    m_device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

    m_device.destroyDescriptorSetLayout(descriptorSetLayout);
}