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

void ToneMapping::Draw(vk::CommandBuffer commandBuffer, std::vector<std::shared_ptr<Framebuffer>> inputFramebuffers, std::shared_ptr<Framebuffer> outputFramebuffer)
{
	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].color.float32[0] = 0.0;
	clearValues[0].color.float32[1] = 0.0;
	clearValues[0].color.float32[2] = 0.0;
	clearValues[0].color.float32[3] = 1.0f;

	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	vk::Extent2D extent(m_width, m_height);
	// Blit offscreen texture to swapchain framebuffer
	// FIXME: Move the following code to tonemapping.cpp.
	vk::ImageSubresourceRange ssr(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
	m_pResourceManager->SetImageLayout(commandBuffer, inputFramebuffers[0]->m_pColorTextures[0]->image, inputFramebuffers[0]->m_pColorTextures[0]->format, ssr,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	m_pResourceManager->SetImageLayout(commandBuffer, inputFramebuffers[1]->m_pColorTextures[0]->image, inputFramebuffers[1]->m_pColorTextures[0]->format, ssr,
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
		pipelineBlit->InitBlit(m_pResourceManager->m_device, outputFramebuffer->m_pRenderPass->Get());
		pipelineBlit->m_bReady = true;
	}

	vk::RenderPassBeginInfo blitRenderPassInfo(
		outputFramebuffer->m_pRenderPass->Get(),
		outputFramebuffer->m_vkFramebuffer,
		vk::Rect2D(vk::Offset2D(0, 0), extent),
		(uint32_t)clearValues.size(),
		clearValues.data());
	commandBuffer.beginRenderPass(&blitRenderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineBlit->GetPipeline());

	vk::Viewport viewport(0, 0, extent.width, extent.height, 0, 1);
	vk::Rect2D sissor(vk::Offset2D(0, 0), extent);
	commandBuffer.setViewport(0, 1, &viewport);
	commandBuffer.setScissor(0, 1, &sissor);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineBlit->GetPipelineLayout(), 0, 1, &(inputFramebuffers[0]->m_dsTexture), 0, nullptr);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineBlit->GetPipelineLayout(), 1, 1, &(inputFramebuffers[1]->m_dsTexture), 0, nullptr);
	commandBuffer.draw(3, 1, 0, 0);

	commandBuffer.endRenderPass();

	m_pResourceManager->SetImageLayout(commandBuffer, inputFramebuffers[1]->m_pColorTextures[0]->image, inputFramebuffers[1]->m_pColorTextures[0]->format, ssr,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);

	m_pResourceManager->SetImageLayout(commandBuffer, inputFramebuffers[0]->m_pColorTextures[0]->image, inputFramebuffers[0]->m_pColorTextures[0]->format, ssr,
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
