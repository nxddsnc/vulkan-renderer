#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "MyMesh.h"
#include "MyMaterial.h"
#include <memory.h>
#include "Platform.h"
#include <glm/glm.hpp>
#include "Utils.h"
#pragma once
/************************************************************************/
/* Minimum renderable node.*/
/************************************************************************/
class Pipeline;

struct VulkanTexture
{
    vk::Image							image;
    VmaAllocation						imageMemory;
    vk::ImageView                       imageView;
    vk::Sampler                         imageSampler;
	std::shared_ptr<MyTexture>			texture;
	vk::Format							format;
	int									referenceCount = 0;
};

class Drawable
{
public:
	Drawable();
	~Drawable();

	void ComputeBBox();
public:
	BBox						  m_bbox;
    glm::mat4                     m_matrix;
    glm::mat4                     m_normalMatrix;
    std::shared_ptr<MyMesh>       m_mesh;
    std::shared_ptr<MyMaterial>   m_material;

    // buffers
    std::vector<vk::Buffer>       m_vertexBuffers;
    std::vector<vk::DeviceSize>   m_vertexBufferOffsets;
    std::vector<VmaAllocation>    m_vertexBufferMemorys;
    vk::Buffer                    m_indexBuffer;
    VmaAllocation                 m_indexBufferMemory;
    std::shared_ptr<Pipeline>     m_pPipeline;
    vk::DescriptorSet             m_materialDescriptorSet;

    // uniforms

    // texture samplers
    std::shared_ptr<VulkanTexture> baseColorTexture;
    std::shared_ptr<VulkanTexture> normalTexture;
    std::shared_ptr<VulkanTexture> metallicRoughnessTexture;
    vk::DescriptorSet              textureDescriptorSet;

	bool						   m_bReady;

    //vk::DescriptorSet              textureDescriptorSet;
};

