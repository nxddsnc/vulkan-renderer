#include "vulkan.hpp"
#pragma once

class VulkanRenderer;
class MyGui
{
public:
	MyGui(VulkanRenderer *renderer);
	~MyGui();

	void Draw(vk::CommandBuffer& commandBuffer);
};

