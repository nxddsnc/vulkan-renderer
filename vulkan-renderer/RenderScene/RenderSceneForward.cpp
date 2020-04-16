#include "RenderSceneForward.h"
#include "Drawable.h"
#include "ResourceManager.h"
#include "PipelineManager.h"
#include "RenderQueue.h"
#include "RenderQueueManager.h"
#include "Camera.hpp"
#include "Skybox.h"
#include "Axis.h"
#include "SHLight.h"
#include "Framebuffer.h"
#include "MyTexture.h"
#include "RenderPass.h"

RenderSceneForward::RenderSceneForward(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height):
	RenderScene(resourceManager, pipelineManager, width, height)
{
	m_pipelineType = PipelineType::MODEL_FORWARD;
	_init();
}


RenderSceneForward::~RenderSceneForward()
{
	_deInit();
}

std::shared_ptr<Framebuffer> RenderSceneForward::GetFramebuffer()
{
	return m_framebuffers[0];
}


void RenderSceneForward::_init()
{
	std::vector<MyImageFormat> colorFormats = { MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT };

	std::shared_ptr<Framebuffer> framebuffer = std::make_shared<Framebuffer>("offscreen-forward", m_pResourceManager,
		colorFormats, MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT, m_width, m_height);

	m_framebuffers.push_back(framebuffer);
}

void RenderSceneForward::_deInit()
{
	m_framebuffers.clear();
}

void RenderSceneForward::_begin(vk::CommandBuffer &commandBuffer)
{
	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].color.float32[0] = 0.0;
	clearValues[0].color.float32[1] = 0.0;
	clearValues[0].color.float32[2] = 0.0;
	clearValues[0].color.float32[3] = 1.0f;

	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	vk::RenderPassBeginInfo renderPassInfo(m_framebuffers[0]->m_pRenderPass->Get(), m_framebuffers[0]->m_vkFramebuffer,
		vk::Rect2D(vk::Offset2D(0, 0),
			vk::Extent2D(m_framebuffers[0]->m_pColorTextures[0]->texture->m_pImage->m_width, m_framebuffers[0]->m_pColorTextures[0]->texture->m_pImage->m_height)),
		(uint32_t)clearValues.size(),
		clearValues.data());

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
}

void RenderSceneForward::_end(vk::CommandBuffer &commandBuffer)
{
	commandBuffer.endRenderPass();
}