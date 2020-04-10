#include "Platform.h"
#pragma once

class ResourceManager;
class MyTexture;
class SHLight
{
public:
    SHLight(ResourceManager *resourceManager, std::vector<std::shared_ptr<MyTexture>> textures);
    ~SHLight();

    void CreateDescriptorSet(vk::Device &device, vk::DescriptorPool &descriptorPool);
    void UpdateUniformBuffer();
private:
    glm::mat4           m_matrixR;
    glm::mat4           m_matrixG;
    glm::mat4           m_matrixB;

	ResourceManager   * m_pResourceManager;

public:
    vk::Buffer          m_uniformBuffer;
    VmaAllocation       m_uniformBufferMemory;
    vk::DescriptorSet   m_descriptorSet;
};
