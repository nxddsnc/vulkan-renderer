#include "SHLight.h"
#include <glm/gtc/type_ptr.hpp>

SHLight::SHLight(VmaAllocator *memoryAllocator)
{
    float r[16] = { 0.09009903, -0.047194730000000004, 0.24026408000000002, -0.14838256, -0.047194730000000004, -0.09009903, -0.11155118, 0.19954896, 0.24026408000000002, -0.11155118, -0.1189, -0.17396576000000002, -0.14838256, 0.19954896, -0.17396576000000002, 0.73975261 };

    float g[16] = { -0.021452150000000003, -0.021452150000000003, 0.09009903, -0.03069984, -0.021452150000000003, 0.021452150000000003, -0.09438946000000001, 0.1790824, 0.09009903, -0.09438946000000001, -0.06688125, -0.09209952, -0.03069984, 0.1790824, -0.09209952, 0.41223360000000003 };
    
    float b[16] = { -0.1287129, -0.05148516, 0.060066020000000005, 0.00511664, -0.05148516, 0.1287129, -0.20165021, 0.3069984, 0.060066020000000005, -0.20165021, -0.11146875, -0.13814928, 0.00511664, 0.3069984, -0.13814928, 0.51571878 };

    m_matrixR = glm::make_mat4(r);
    m_matrixG = glm::make_mat4(g);
    m_matrixB = glm::make_mat4(b);

    m_pMemoryAllocator = memoryAllocator;

    vk::BufferCreateInfo createInfo({ {},
        vk::DeviceSize(sizeof(m_matrixR) * 3),
        vk::BufferUsageFlagBits::eUniformBuffer,
        {},
        {},
        {} });
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    VkBufferCreateInfo &vkCreateInfo = createInfo;
    vmaCreateBuffer(*m_pMemoryAllocator, &vkCreateInfo, &allocInfo, reinterpret_cast<VkBuffer*>(&m_uniformBuffer), &m_uniformBufferMemory, nullptr);
}

SHLight::~SHLight()
{
    vmaDestroyBuffer(*m_pMemoryAllocator, m_uniformBuffer, m_uniformBufferMemory);
}

void SHLight::CreateDescriptorSet(vk::Device &device, vk::DescriptorPool &descriptorPool)
{
    vk::DescriptorSetLayout descriptorSetLayout;
    // light uniform buffer
    vk::DescriptorSetLayoutBinding lightBinding({ 0,
        vk::DescriptorType::eUniformBuffer,
        1,
        vk::ShaderStageFlagBits::eFragment,
        {} });

    vk::DescriptorSetLayoutCreateInfo layoutInfo({ {},
        1,
        &lightBinding });
    descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

    vk::DescriptorSetAllocateInfo allocInfo({
        descriptorPool,
        1,
        &descriptorSetLayout
    });
    device.allocateDescriptorSets(&allocInfo, &m_descriptorSet);

    vk::DescriptorBufferInfo bufferInfo({
        m_uniformBuffer,
        0,
        sizeof(m_matrixR) * 3
    });

    vk::WriteDescriptorSet descriptorWrite({
        m_descriptorSet,
        0,
        0,
        1,
        vk::DescriptorType::eUniformBuffer,
        {},
        &bufferInfo,
        {}
    });
    device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

    device.destroyDescriptorSetLayout(descriptorSetLayout);
}

void SHLight::UpdateUniformBuffer()
{
    char* data = nullptr;
    vmaMapMemory(*m_pMemoryAllocator, m_uniformBufferMemory, reinterpret_cast<void**>(&data));
    memcpy(data, &m_matrixR, sizeof(m_matrixR));
    memcpy(data + sizeof(m_matrixR), &m_matrixG, sizeof(m_matrixG));
    memcpy(data + 2 * sizeof(m_matrixR), &m_matrixB, sizeof(m_matrixB));
    vmaUnmapMemory(*m_pMemoryAllocator, m_uniformBufferMemory);
}

