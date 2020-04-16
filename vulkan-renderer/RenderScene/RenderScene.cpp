#include "RenderScene.h"
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
#include "RenderPass.h"
#include "MyTexture.h"

RenderScene::RenderScene(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height)
{
	m_pResourceManager = resourceManager;
	m_pPipelineManager = pipelineManager;
	m_width			   = width;
	m_height		   = height;

	m_pRenderQueueManager = std::make_shared<RenderQueueManager>(pipelineManager);

	m_pCamera = std::make_shared<VulkanCamera>(&(resourceManager->m_memoryAllocator));
	m_pCamera->type = VulkanCamera::CameraType::lookat;
	m_pCamera->setPosition(glm::vec3(0, 0, 0));
	m_pCamera->setRotation(glm::vec3(-45, 0, 45));
	m_pCamera->setPerspective(45.0f, (float)width / (float)height, 0.1f, 10.0f);

	m_pCamera->createDescriptorSet(m_pResourceManager->m_device, m_pResourceManager->m_descriptorPool);
}

RenderScene::~RenderScene()
{
}

void RenderScene::AddRenderNodes(std::vector<std::shared_ptr<Drawable>> drawables)
{
	for (int i = 0; i < drawables.size(); ++i)
	{
		std::shared_ptr<Drawable> drawable = drawables[i];
		m_pResourceManager->InitVulkanResource(drawable);

		PipelineId id;

		id.model.primitivePart.info.bits.positionVertexData = 1;
		id.model.primitivePart.info.bits.normalVertexData = 1;
		id.model.primitivePart.info.bits.tangentVertexData = drawable->m_mesh->m_vertexBits.hasTangent;
		id.model.primitivePart.info.bits.countTexCoord = drawable->m_mesh->m_vertexBits.hasTexCoord0 ? 1 : 0;
		id.model.primitivePart.info.bits.countColor = drawable->m_mesh->m_vertexBits.hasColor;
		id.model.materialPart.info.bits.baseColorInfo = 1;
		id.model.materialPart.info.bits.baseColorMap = drawable->m_material->m_pDiffuseMap != nullptr;
		id.model.materialPart.info.bits.normalMap = drawable->m_material->m_pNormalMap != nullptr;
		id.type = m_pipelineType;

		auto renderQueue = m_pRenderQueueManager->GetRenderQueue(id, m_framebuffers[0]->m_pRenderPass);
		renderQueue->AddDrawable(drawable);
	}
}

void RenderScene::Draw(vk::CommandBuffer& commandBuffer)
{
	_begin(commandBuffer);
	// drawAxis
	m_pAxis->CreateDrawCommand(commandBuffer, m_pCamera->descriptorSet, m_framebuffers[0]->m_pRenderPass);

	for (auto renderQueue : m_pRenderQueueManager->m_renderQueues)
	{
		renderQueue->Draw(commandBuffer, m_pCamera, m_pSkybox);
	}
	
	m_pSkybox->Draw(commandBuffer, m_pCamera, m_framebuffers[0]->m_pRenderPass);

	_end(commandBuffer);
}

void RenderScene::UpdateUniforms()
{
	m_pCamera->UpdateUniformBuffer();
	m_pSkybox->m_pSHLight->UpdateUniformBuffer();
}

std::shared_ptr<Framebuffer> RenderScene::GetFramebuffer()
{
	return m_framebuffers[0];
}

void RenderScene::_begin(vk::CommandBuffer &commandBuffer)
{
	
}

void RenderScene::_end(vk::CommandBuffer &commandBuffer)
{

}
