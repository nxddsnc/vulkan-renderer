#include "Platform.h"
#include <vector>

struct DescriptorSet
{
    int         index;
    
}
class DescriptorSets
{
public:
    DescriptorSets();
    ~DescriptorSets();

    Init();
private:
    const vk::Device& _device;
    const vk::DescriptorPool& _descriptorPool;
    std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
    vk::DescriptorSet _descriptorSet;
}