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

struct Drawable
{
    glm::mat4                     matrix;
    glm::mat4                     normalMatrix;
    std::shared_ptr<MyMesh>       mesh;
    std::shared_ptr<MyMaterial>   material;

    // buffers
    std::vector<vk::Buffer>       vertexBuffers;
    std::vector<vk::DeviceSize>   vertexBufferOffsets;
    std::vector<VmaAllocation>    vertexBufferMemorys;
    vk::Buffer                    indexBuffer;
    VmaAllocation                 indexBufferMemory;
    std::shared_ptr<Pipeline>     pipeline;
    vk::DescriptorSet             materialDescriptorSet;
    vk::DescriptorSet             materialImageDescriptorSet;

    // uniforms

    // texture samplers
    std::shared_ptr<VulkanTexture> baseColorTexture;
    std::shared_ptr<VulkanTexture> normalTexture;

    vk::DescriptorSet              textureDescriptorSet;

    //vk::DescriptorSet              textureDescriptorSet;
};

