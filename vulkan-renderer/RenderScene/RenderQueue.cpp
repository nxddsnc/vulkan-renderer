#include "RenderQueue.h"
#include "Renderable.h"
#include "Pipeline.h"
#include "MyCamera.h"
#include "Skybox.h"
#include "SHLight.h"
#include "ShadowMap.h"
#include "MyCamera.h"
#include "MyAnimation.h"

RenderQueue::RenderQueue(std::shared_ptr<Pipeline> pipeline)
{
	m_pPipeline = pipeline;
}


RenderQueue::~RenderQueue()
{
}

void RenderQueue::AddRenderable(std::shared_ptr<Renderable> renderable)
{
	renderable->m_pPipeline = m_pPipeline;
	m_renderables.push_back(renderable);
}

void RenderQueue::Draw(vk::CommandBuffer commandBuffer, std::shared_ptr<MyCamera> camera, std::shared_ptr<Skybox> skybox, std::shared_ptr<ShadowMap> shadowMap, int width, int height)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipeline());

	if (width != 0 && height != 0)
	{
		vk::Viewport viewport(0, 0, width, height, 0, 1);
		vk::Rect2D sissor(vk::Offset2D(0, 0), vk::Extent2D(width, height));
		commandBuffer.setViewport(0, 1, &viewport);
		commandBuffer.setScissor(0, 1, &sissor);
	}

	for (auto renderable : m_renderables)
	{
		renderable->Render(commandBuffer, m_pPipeline.get(), camera.get());
	}
}
