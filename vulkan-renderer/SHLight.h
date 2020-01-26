#include "Platform.h"
#pragma once

class SHLight
{
public:
    SHLight(VmaAllocator *memoryAllocator);
    ~SHLight();

    void CreateDescriptorSet(vk::Device &device, vk::DescriptorPool &descriptorPool);
    void UpdateUniformBuffer();
private:
    glm::mat4           m_matrixR;
    glm::mat4           m_matrixG;
    glm::mat4           m_matrixB;

    VmaAllocator      * m_pMemoryAllocator;

public:
    vk::Buffer          m_uniformBuffer;
    VmaAllocation       m_uniformBufferMemory;
    vk::DescriptorSet   m_descriptorSet;
};
