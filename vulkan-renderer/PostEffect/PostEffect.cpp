#include "PostEffect.h"
#include "ResourceManager.h"
#include "PipelineManager.h"
#include "Framebuffer.h"
#include "MyTexture.h"
#include "Drawable.h"

PostEffect::PostEffect(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height)
{
	m_pResourceManager = resourceManager;
	m_pPipelineManager = pipelineManager;
	m_width = width;
	m_height = height;

	_init();
}

PostEffect::~PostEffect()
{
	_deInit();
}

void PostEffect::Draw(vk::CommandBuffer commandBuffer, std::shared_ptr<Framebuffer> inputFramebuffer, std::shared_ptr<Framebuffer> outputFramebuffer)
{
	
}

std::shared_ptr<Framebuffer> PostEffect::GetFramebuffer()
{
	return nullptr;
}

void PostEffect::_init()
{

}

void PostEffect::_deInit()
{

}

void PostEffect::beforeRendering(vk::CommandBuffer& commandBuffer,std::shared_ptr<Framebuffer> inputFramebuffer, vk::ImageSubresourceRange& ssr)
{
	
}

void PostEffect::afterRendering(vk::CommandBuffer& commandBuffer, std::shared_ptr<Framebuffer> inputFramebuffer, vk::ImageSubresourceRange& ssr)
{

}
