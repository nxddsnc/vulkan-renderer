#include "RenderScene.h"
#pragma once

class RenderSceneForward : public RenderScene
{
public:
	RenderSceneForward(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height);
	~RenderSceneForward();

	std::shared_ptr<Framebuffer> GetFramebuffer();
private:
	void _init();
	void _deInit();

	void _begin(vk::CommandBuffer &commandBuffer);
	void _end(vk::CommandBuffer &commandBuffer);
};
