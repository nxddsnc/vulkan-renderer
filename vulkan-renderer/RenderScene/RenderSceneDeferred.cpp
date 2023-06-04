#include "RenderSceneDeferred.h"
#include "Renderable.h"
#include "ResourceManager.h"
#include "PipelineManager.h"
#include "RenderQueue.h"
#include "RenderQueueManager.h"
#include "MyCamera.h"
#include "Skybox.h"
#include "Axis.h"
#include "SHLight.h"
#include "Framebuffer.h"
#include "MyTexture.h"
#include "RenderPass.h"
#include "ShadowMap.h"

RenderSceneDeferred::RenderSceneDeferred(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height) :
	RenderScene(resourceManager, pipelineManager, width, height)
{
	m_pipelineType = PipelineType::MODEL_DEFERRED;
	_init();
}


RenderSceneDeferred::~RenderSceneDeferred()
{
	_deInit();
}

std::shared_ptr<Framebuffer> RenderSceneDeferred::GetFramebuffer()
{
	return m_outputFramebuffer;
}

void RenderSceneDeferred::Draw(vk::CommandBuffer& commandBuffer)
{
	m_pShadowMap->Draw(commandBuffer);

	_begin(commandBuffer);
	// drawAxis
	//m_pAxis->CreateDrawCommand(commandBuffer, m_pCamera->m_descriptorSet, m_framebuffers[0]->m_pRenderPass);

	for (auto renderQueue : m_renderQueues)
	{
		renderQueue->Draw(commandBuffer, m_pCamera, m_pSkybox);
	}

	//m_pSkybox->Draw(commandBuffer, m_pCamera, m_framebuffers[0]->m_pRenderPass);

	_end(commandBuffer);
     
	_doShading(commandBuffer);

}

void RenderSceneDeferred::_init()
{
	std::vector<MyImageFormat> colorFormats = { MyImageFormat::MY_IMAGEFORMAT_RGBA32_FLOAT, // position 
												MyImageFormat::MY_IMAGEFORMAT_RGBA32_FLOAT, // normal
												MyImageFormat::MY_IMAGEFORMAT_RGBA32_FLOAT }; // albedo

	std::shared_ptr<Framebuffer> framebuffer = std::make_shared<Framebuffer>("deferred-mrt-color", "deferred-depth", m_pResourceManager,
		colorFormats, MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT, m_width, m_height);

	m_framebuffers.push_back(framebuffer);

	std::vector<MyImageFormat> outputColorFormats = { MyImageFormat::MY_IMAGEFORMAT_RGBA32_FLOAT };
	 m_outputFramebuffer = std::make_shared<Framebuffer>("deferred-output-color", "deferred-depth", m_pResourceManager,
		 outputColorFormats, MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT, m_width, m_height);

	 PipelineId id;
	 id.type = PipelineType::DEFERRED_SHADING;
	 id.model.primitivePart.info.bits.positionVertexData = 1;
	 id.model.primitivePart.info.bits.normalVertexData = 0;
	 id.model.primitivePart.info.bits.countTexCoord = 1;
	 id.model.primitivePart.info.bits.tangentVertexData = 0;
	 id.model.primitivePart.info.bits.countColor = 0;
	 id.model.primitivePart.info.bits.primitiveMode = PrimitiveMode::Triangles;

	 m_pPipeline = m_pPipelineManager->GetPipeline(id, m_outputFramebuffer->m_pRenderPass);
}

void RenderSceneDeferred::_deInit()
{
	m_framebuffers.clear();
}

void RenderSceneDeferred::_begin(vk::CommandBuffer &commandBuffer)
{
	std::array<vk::ClearValue, 4> clearValues{};
	clearValues[0].color.float32[0] = 0.0;
	clearValues[0].color.float32[1] = 0.0;
	clearValues[0].color.float32[2] = 0.0;
	clearValues[0].color.float32[3] = 1.0f;

	clearValues[1].color.float32[0] = 0.0;
	clearValues[1].color.float32[1] = 0.0;
	clearValues[1].color.float32[2] = 0.0;
	clearValues[1].color.float32[3] = 1.0f;

	clearValues[2].color.float32[0] = 0.0;
	clearValues[2].color.float32[1] = 0.0;
	clearValues[2].color.float32[2] = 0.0;
	clearValues[2].color.float32[3] = 1.0f;

	clearValues[3].depthStencil.depth = 1.0f;
	clearValues[3].depthStencil.stencil = 0;

	vk::RenderPassBeginInfo renderPassInfo(m_framebuffers[0]->m_pRenderPass->Get(), m_framebuffers[0]->m_vkFramebuffer,
		vk::Rect2D(vk::Offset2D(0, 0),
			vk::Extent2D(m_framebuffers[0]->m_pColorTextures[0]->texture->m_pImage->m_width, m_framebuffers[0]->m_pColorTextures[0]->texture->m_pImage->m_height)),
		(uint32_t)clearValues.size(),
		clearValues.data());

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
}

void RenderSceneDeferred::_end(vk::CommandBuffer &commandBuffer)
{
	commandBuffer.endRenderPass();
}

void RenderSceneDeferred::_doShading(vk::CommandBuffer & commandBuffer)
{
	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].color.float32[0] = 0.0;
	clearValues[0].color.float32[1] = 0.0;
	clearValues[0].color.float32[2] = 0.0;
	clearValues[0].color.float32[3] = 1.0f;

	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	vk::ImageSubresourceRange ssr(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
	vk::ImageSubresourceRange ssrDepth(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1);

	m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffers[0]->m_pColorTextures[0]->image, m_framebuffers[0]->m_pColorTextures[0]->format, ssr,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffers[0]->m_pColorTextures[1]->image, m_framebuffers[0]->m_pColorTextures[1]->format, ssr,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffers[0]->m_pColorTextures[2]->image, m_framebuffers[0]->m_pColorTextures[2]->format, ssr,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	//m_pResourceManager->SetImageLayout(commandBuffer, m_pShadowMap->m_pFramebuffer->m_pColorTextures[0]->image, m_pShadowMap->m_pFramebuffer->m_pColorTextures[0]->format, ssr,
	//	vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	m_pResourceManager->SetImageLayout(commandBuffer, m_pShadowMap->m_pFramebuffer->m_pDepthTexture->image, m_pShadowMap->m_pFramebuffer->m_pDepthTexture->format, ssrDepth,
		vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	vk::RenderPassBeginInfo renderPassInfo(
		m_outputFramebuffer->m_pRenderPass->Get(),
		m_outputFramebuffer->m_vkFramebuffer,
		vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(m_width, m_height)),
		(uint32_t)clearValues.size(),
		clearValues.data());
	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipeline());

	vk::Viewport viewport(0, 0, m_width, m_height, 0, 1);
	vk::Rect2D sissor(vk::Offset2D(0, 0), vk::Extent2D(m_width, m_height));
	commandBuffer.setViewport(0, 1, &viewport);
	commandBuffer.setScissor(0, 1, &sissor);

	//commandBuffer.pushConstants(pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof(float) * 4, reinterpret_cast<void*>(&m_parameters));
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 0, 1, &m_pCamera->m_descriptorSet, 0, nullptr);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 1, 1, &m_pSkybox->m_pSHLight->m_descriptorSet, 0, nullptr);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 2, 1, &m_pSkybox->m_preFilteredDescriptorSet, 0, nullptr);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 3, 1, &m_framebuffers[0]->m_dsTexture, 0, nullptr);

	if (m_pShadowMap)
	{
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 4, 1, &m_pShadowMap->m_pFramebuffer->m_dsTexture, 0, nullptr);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 5, 1, &m_pShadowMap->m_pCamera->m_descriptorSet, 0, nullptr);
	}

	commandBuffer.draw(3, 1, 0, 0);

	m_pSkybox->Draw(commandBuffer, m_pCamera, m_outputFramebuffer->m_pRenderPass);

	commandBuffer.endRenderPass();

	/*vk::ImageBlit imageBlit(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 0, 1),
	{ vk::Offset3D(0, 0, 0), vk::Offset3D(m_width, m_height, 1) },
		vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 0, 1),
		{ vk::Offset3D(0, 0, 0), vk::Offset3D(m_width, m_height, 1) });

	std::array<vk::ImageBlit, 1> imageBlits = { imageBlit };

	vk::ImageSubresourceRange ssrDepth(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1);
	m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffers[0]->m_pDepthTexture->image, m_framebuffers[0]->m_pDepthTexture->format, ssrDepth,
		vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);

	m_pResourceManager->SetImageLayout(commandBuffer, m_outputFramebuffer->m_pDepthTexture->image, m_outputFramebuffer->m_pDepthTexture->format, ssrDepth,
		vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eTransferDstOptimal);

	commandBuffer.blitImage(m_framebuffers[0]->m_pDepthTexture->image, vk::ImageLayout::eTransferSrcOptimal,
		m_outputFramebuffer->m_pDepthTexture->image, vk::ImageLayout::eTransferDstOptimal, imageBlits, vk::Filter::eNearest);

	m_pResourceManager->SetImageLayout(commandBuffer, m_outputFramebuffer->m_pDepthTexture->image, m_outputFramebuffer->m_pDepthTexture->format, ssrDepth,
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffers[0]->m_pDepthTexture->image, m_framebuffers[0]->m_pDepthTexture->format, ssrDepth,
		vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

	m_pSkybox->Draw(commandBuffer, m_pCamera, m_outputFramebuffer->m_pRenderPass);

	commandBuffer.endRenderPass();*/
	m_pResourceManager->SetImageLayout(commandBuffer, m_pShadowMap->m_pFramebuffer->m_pDepthTexture->image, m_pShadowMap->m_pFramebuffer->m_pDepthTexture->format, ssrDepth,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	//m_pResourceManager->SetImageLayout(commandBuffer, m_pShadowMap->m_pFramebuffer->m_pColorTextures[0]->image, m_pShadowMap->m_pFramebuffer->m_pColorTextures[0]->format, ssr,
	//	vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
	m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffers[0]->m_pColorTextures[0]->image, m_framebuffers[0]->m_pColorTextures[0]->format, ssr,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
	m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffers[0]->m_pColorTextures[1]->image, m_framebuffers[0]->m_pColorTextures[1]->format, ssr,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
	m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffers[0]->m_pColorTextures[2]->image, m_framebuffers[0]->m_pColorTextures[2]->format, ssr,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
}
