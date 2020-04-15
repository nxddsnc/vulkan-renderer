#include "Bloom.h"
#include "ResourceManager.h"
#include "Framebuffer.h"
#include "PipelineManager.h"
#include "MyImage.h"
#include "Drawable.h"
#include "MyTexture.h"
#include "RenderPass.h"

Bloom::Bloom(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height) :
	PostEffect(resourceManager, pipelineManager, width, height)
{
	m_parameters[0] = 1.0;
	m_parameters[1] = 1.5;
	m_parameters[2] = 0.0;
	m_parameters[3] = 0.0;

	m_brightnessThreshold = 1.0;
	_init();
}

Bloom::~Bloom()
{
	_deInit();
}

void Bloom::Draw(vk::CommandBuffer commandBuffer, std::shared_ptr<Framebuffer> inputFramebuffer, std::shared_ptr<Framebuffer> outputFramebuffer)
{
	vk::ImageSubresourceRange ssr(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].color.float32[0] = 0.0;
	clearValues[0].color.float32[1] = 0.0;
	clearValues[0].color.float32[2] = 0.0;
	clearValues[0].color.float32[3] = 1.0f;

	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	// TODO: ImageLayout should become a property of vulkanTexture.
	m_pResourceManager->SetImageLayout(commandBuffer, inputFramebuffer->m_pColorTextures[0]->image, inputFramebuffer->m_pColorTextures[0]->format, ssr,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	
	// bright pass filter
	vk::RenderPassBeginInfo brightnessPassInfo(
		m_framebuffer1->m_pRenderPass->Get(),
		m_framebuffer1->m_vkFramebuffer,
		vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(m_width, m_height)),
		(uint32_t)clearValues.size(),
		clearValues.data());
	commandBuffer.beginRenderPass(&brightnessPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pPipelineBrightness->GetPipeline());

	vk::Viewport viewport(0, 0, m_width, m_height, 0, 1);
	vk::Rect2D sissor(vk::Offset2D(0, 0), vk::Extent2D(m_width, m_height));
	commandBuffer.setViewport(0, 1, &viewport);
	commandBuffer.setScissor(0, 1, &sissor);

	commandBuffer.pushConstants(m_pPipelineBrightness->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof(float), reinterpret_cast<void*>(&m_brightnessThreshold));
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipelineBrightness->GetPipelineLayout(), 0, 1, &(inputFramebuffer->m_dsTexture), 0, nullptr);
	commandBuffer.draw(3, 1, 0, 0);

	commandBuffer.endRenderPass();

	m_pResourceManager->SetImageLayout(commandBuffer, inputFramebuffer->m_pColorTextures[0]->image, inputFramebuffer->m_pColorTextures[0]->format, ssr,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);


	for (int i = 0; i < 2; ++i)
	{
		// blur x
		m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffer1->m_pColorTextures[0]->image, m_framebuffer1->m_pColorTextures[0]->format, ssr,
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::RenderPassBeginInfo blurPassInfoX(
			m_framebuffer2->m_pRenderPass->Get(),
			m_framebuffer2->m_vkFramebuffer,
			vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(m_width, m_height)),
			(uint32_t)clearValues.size(),
			clearValues.data());
		commandBuffer.beginRenderPass(&blurPassInfoX, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pPipelineBlurX->GetPipeline());

		commandBuffer.setViewport(0, 1, &viewport);
		commandBuffer.setScissor(0, 1, &sissor);

		m_parameters[2] = 1.0;
		m_parameters[3] = 0.0;
		commandBuffer.pushConstants(m_pPipelineBlurX->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof(float) * 4, reinterpret_cast<void*>(&m_parameters));
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipelineBlurX->GetPipelineLayout(), 0, 1, &(m_framebuffer1->m_dsTexture), 0, nullptr);
		commandBuffer.draw(3, 1, 0, 0);

		commandBuffer.endRenderPass();

		m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffer1->m_pColorTextures[0]->image, m_framebuffer1->m_pColorTextures[0]->format, ssr,
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);

		// blur y
		m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffer2->m_pColorTextures[0]->image, m_framebuffer2->m_pColorTextures[0]->format, ssr,
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::RenderPassBeginInfo blurPassInfoY(
			m_framebuffer1->m_pRenderPass->Get(),
			m_framebuffer1->m_vkFramebuffer,
			vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(m_width, m_height)),
			(uint32_t)clearValues.size(),
			clearValues.data());
		commandBuffer.beginRenderPass(&blurPassInfoY, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pPipelineBlurY->GetPipeline());

		commandBuffer.setViewport(0, 1, &viewport);
		commandBuffer.setScissor(0, 1, &sissor);

		m_parameters[2] = 0.0;
		m_parameters[3] = 1.0;
		commandBuffer.pushConstants(m_pPipelineBlurY->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof(float) * 4, reinterpret_cast<void*>(&m_parameters));
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipelineBlurY->GetPipelineLayout(), 0, 1, &(m_framebuffer2->m_dsTexture), 0, nullptr);
		commandBuffer.draw(3, 1, 0, 0);

		commandBuffer.endRenderPass();

		m_pResourceManager->SetImageLayout(commandBuffer, m_framebuffer2->m_pColorTextures[0]->image, m_framebuffer2->m_pColorTextures[0]->format, ssr,
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
	}
	

	PostEffect::afterRendering(commandBuffer, inputFramebuffer, ssr);
}

std::shared_ptr<Framebuffer> Bloom::GetFramebuffer()
{
	return m_framebuffer1;
}

void Bloom::_init()
{
	std::vector<MyImageFormat> colorFormats = { MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT };
	m_framebuffer1 = std::make_shared<Framebuffer>("bloom-1", m_pResourceManager,
		colorFormats, MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT,
		m_width, m_height);

	m_framebuffer2 = std::make_shared<Framebuffer>("bloom-2", m_pResourceManager,
		colorFormats, MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT,
		m_width, m_height);

	PipelineId brightnessPipelineId;
	brightnessPipelineId.type = PipelineType::BRIGHTNESS;
	brightnessPipelineId.model.primitivePart.info.bits.positionVertexData = 1;
	brightnessPipelineId.model.primitivePart.info.bits.normalVertexData = 0;
	brightnessPipelineId.model.primitivePart.info.bits.countTexCoord = 1;
	brightnessPipelineId.model.primitivePart.info.bits.tangentVertexData = 0;
	brightnessPipelineId.model.primitivePart.info.bits.countColor = 0;

	m_pPipelineBrightness = m_pPipelineManager->GetPipeline(brightnessPipelineId);
	m_pPipelineBrightness->InitBrightPass(m_pResourceManager->m_device, m_framebuffer1->m_pRenderPass->Get());
	m_pPipelineBrightness->m_bReady = true;

	PipelineId blurPipelineId;
	blurPipelineId.type = PipelineType::GAUSSIAN_BLUR_X;
	blurPipelineId.model.primitivePart.info.bits.positionVertexData = 1;
	blurPipelineId.model.primitivePart.info.bits.normalVertexData = 0;
	blurPipelineId.model.primitivePart.info.bits.countTexCoord = 1;
	blurPipelineId.model.primitivePart.info.bits.tangentVertexData = 0;
	blurPipelineId.model.primitivePart.info.bits.countColor = 0;

	m_pPipelineBlurX = m_pPipelineManager->GetPipeline(blurPipelineId);
	m_pPipelineBlurX->InitGaussianBlur(m_pResourceManager->m_device, m_framebuffer2->m_pRenderPass->Get());
	m_pPipelineBlurX->m_bReady = true;

	blurPipelineId.type = PipelineType::GAUSSIAN_BLUR_Y;

	m_pPipelineBlurY = m_pPipelineManager->GetPipeline(blurPipelineId);
	m_pPipelineBlurY->InitGaussianBlur(m_pResourceManager->m_device, m_framebuffer1->m_pRenderPass->Get());
	m_pPipelineBlurY->m_bReady = true;
}

void Bloom::_deInit()
{
	m_framebuffer1 = nullptr;
	m_framebuffer2 = nullptr;
}
