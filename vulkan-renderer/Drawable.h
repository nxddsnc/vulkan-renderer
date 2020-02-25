#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "MyMesh.h"
#include "MyMaterial.h"
#include <memory.h>
#include "Platform.h"
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
};

class Drawable
{
public:
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

    //vk::DescriptorSet              textureDescriptorSet;
};

