#include "PostEffect.h"
#include "ResourceManager.h"
#include "PipelineManager.h"
#include "Framebuffer.h"

PostEffect::PostEffect(ResourceManager *resourceManager, PipelineManager *pipelineManager, std::shared_ptr<Framebuffer> inputFramebuffer)
{
	m_pResourceManager = resourceManager;
	m_pPipelineManager = pipelineManager;
	m_pInputFramebuffer = inputFramebuffer;
}

PostEffect::~PostEffect()
{
}

void PostEffect::Draw(vk::CommandBuffer commandBuffer)
{

}
