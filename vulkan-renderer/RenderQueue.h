#include "Platform.h"

#pragma once
class Drawable;
class Pipeline;
class VulkanCamera;
class Skybox;
class RenderQueue
{
public:
	RenderQueue(std::shared_ptr<Pipeline> pipeline);
	~RenderQueue();

	void AddDrawable(std::shared_ptr<Drawable> drawable);
	void Draw(vk::CommandBuffer commandBuffer, std::shared_ptr<VulkanCamera> camera, std::shared_ptr<Skybox> skybox);
private:
	std::vector<std::shared_ptr<Drawable>>			m_drawables;
	std::shared_ptr<Pipeline>						m_pPipeline;
};

