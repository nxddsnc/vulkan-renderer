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

class MyAnimation;

enum DRAWALBE_TYPE
{
	SINGLE_DRAWABLE,
	INSTANCE_DRAWABLE
};
class Drawable
{
public:
	virtual void ComputeBBox() = 0;

	int GetHash();
	BBox						  m_bbox;


	std::shared_ptr<MyMesh>       m_mesh;
	std::shared_ptr<MyMaterial>   m_material;
	std::shared_ptr<MyAnimation>   m_pAnimation;
	DRAWALBE_TYPE				   m_type;

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
};

class SingleDrawable : public Drawable
{
public:
	SingleDrawable();
	~SingleDrawable();

	void ComputeBBox();
public:
    glm::mat4                     m_matrix;
    glm::mat4                     m_normalMatrix;
};

class InstanceDrawable : public Drawable
{
public:
	InstanceDrawable();
	InstanceDrawable(std::shared_ptr<SingleDrawable> d);
	~InstanceDrawable();

	void AddDrawable(std::shared_ptr<SingleDrawable> d);
	void ComputeBBox();
public:
	std::vector<glm::mat4>  m_matricies;
	std::vector<glm::mat4>  m_normalMatrices;


	std::vector<vk::Buffer>       m_instanceBuffer;
	std::vector<VmaAllocation>    m_instanceBufferMemory;

};
