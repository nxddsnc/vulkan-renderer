#include "vulkan.hpp"
#pragma once

class VulkanRenderer;
class MyGui
{
public:
	MyGui(VulkanRenderer *renderer);
	~MyGui();

	void Draw(vk::CommandBuffer& commandBuffer);

    void Clear();

    void AddInfo(std::string info);
private:
    std::vector<std::string> _graphicsInfos;
};

