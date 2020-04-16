#include "RenderScene.h"
#pragma once

class RenderSceneDeferred : public RenderScene
{
public:
	RenderSceneDeferred(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height);
	~RenderSceneDeferred();

	std::shared_ptr<Framebuffer> GetFramebuffer();

	void Draw(vk::CommandBuffer& commandBuffer);

public:
	std::shared_ptr<Framebuffer>										m_outputFramebuffer;

private:
	void _init();
	void _deInit();

	void _begin(vk::CommandBuffer &commandBuffer);
	void _end(vk::CommandBuffer &commandBuffer);

	void _doShading(vk::CommandBuffer& commandBuffer);

private:
	std::shared_ptr<Pipeline> m_pPipeline;
};
