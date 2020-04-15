#include "Platform.h"
#pragma once
class ResourceManager;
class PipelineManager;
class Framebuffer;
class PostEffect
{
public:
	PostEffect(ResourceManager *resourecManager, PipelineManager *pipelineManager, int width, int height);
	virtual ~PostEffect();

	virtual void Draw(vk::CommandBuffer commandBuffer, std::vector<std::shared_ptr<Framebuffer>> inputFramebuffers, std::shared_ptr<Framebuffer> outputFramebuffer = nullptr);

	virtual std::shared_ptr<Framebuffer> GetFramebuffer();

protected:
	ResourceManager					*m_pResourceManager;
	PipelineManager					*m_pPipelineManager;

	int								 m_width;
	int							     m_height;

private: 
	virtual void _init();
	virtual void _deInit();

protected:
	virtual void PostEffect::beforeRendering(vk::CommandBuffer& commandBuffer, std::shared_ptr<Framebuffer> inputFramebuffer, vk::ImageSubresourceRange& ssr);
	virtual void PostEffect::afterRendering(vk::CommandBuffer& commandBuffer, std::shared_ptr<Framebuffer> inputFramebuffer, vk::ImageSubresourceRange& ssr);
};