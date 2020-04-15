#include "ToneMapping.h"
#include "ResourceManager.h"
#include "Framebuffer.h"
#include "PipelineManager.h"
#include "MyImage.h"
#include "Drawable.h"
#include "MyTexture.h"
#include "RenderPass.h"

ToneMapping::ToneMapping(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height) :
	PostEffect(resourceManager, pipelineManager, width, height)
{
	_init();
}

ToneMapping::~ToneMapping()
{
	_deInit();
}

void ToneMapping::Draw(vk::CommandBuffer commandBuffer, std::shared_ptr<Framebuffer> inputFramebuffer, std::shared_ptr<Framebuffer> outputFramebuffer)
{
	vk::ImageSubresourceRange ssr(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
	m_pResourceManager->SetImageLayout(commandBuffer, inputFramebuffer->m_pColorTextures[0]->image, inputFramebuffer->m_pColorTextures[0]->format, ssr,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	PipelineId blitPipelineId;
	blitPipelineId.type = PipelineType::BLIT;
	blitPipelineId.model.primitivePart.info.bits.positionVertexData = 1;
	blitPipelineId.model.primitivePart.info.bits.normalVertexData = 0;
	blitPipelineId.model.primitivePart.info.bits.countTexCoord = 1;
	blitPipelineId.model.primitivePart.info.bits.tangentVertexData = 0;
	blitPipelineId.model.primitivePart.info.bits.countColor = 0;

	std::shared_ptr<Pipeline> pipelineBlit = m_pPipelineManager->GetPipeline(blitPipelineId);
	if (!pipelineBlit->m_bReady)
	{
		// TODO: refactor
		pipelineBlit->InitBlit(m_pResourceManager->m_device, m_framebuffer->m_pRenderPass->Get());
		pipelineBlit->m_bReady = true;
	}

	vk::RenderPassBeginInfo blitRenderPassInfo(
		m_framebuffer->m_pRenderPass->Get(),
		m_framebuffer->m_vkFramebuffer,
		vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(m_width, m_height)),
		(uint32_t)0,
		nullptr);
	commandBuffer.beginRenderPass(&blitRenderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineBlit->GetPipeline());

	vk::Viewport viewport(0, 0, m_width, m_height, 0, 1);
	vk::Rect2D sissor(vk::Offset2D(0, 0), vk::Extent2D(m_width, m_height));
	commandBuffer.setViewport(0, 1, &viewport);
	commandBuffer.setScissor(0, 1, &sissor);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineBlit->GetPipelineLayout(), 0, 1, &(inputFramebuffer->m_dsTexture), 0, nullptr);
	commandBuffer.draw(3, 1, 0, 0);

	commandBuffer.endRenderPass();

	m_pResourceManager->SetImageLayout(commandBuffer, inputFramebuffer->m_pColorTextures[0]->image, inputFramebuffer->m_pColorTextures[0]->format, ssr,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
}

std::shared_ptr<Framebuffer> ToneMapping::GetFramebuffer()
{
	return m_framebuffer;
}

void ToneMapping::_init()
{

}

void ToneMapping::_deInit()
{

}
