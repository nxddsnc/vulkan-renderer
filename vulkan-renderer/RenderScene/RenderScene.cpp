#include "RenderScene.h"
#include "Drawable.h"
#include "ResourceManager.h"
#include "PipelineManager.h"
#include "RenderQueue.h"
#include "RenderQueueManager.h"
#include "MyCamera.h"
#include "Skybox.h"
#include "Axis.h"
#include "SHLight.h"
#include "Framebuffer.h"
#include "RenderPass.h"
#include "MyTexture.h"
#include "MyScene.h"
#include "ShadowMap.h"

RenderScene::RenderScene(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height)
{
	m_pResourceManager = resourceManager;
	m_pPipelineManager = pipelineManager;
	m_width			   = width;
	m_height		   = height;

	m_pRenderQueueManager = std::make_shared<RenderQueueManager>(pipelineManager);

	m_pCamera = std::make_shared<MyCamera>(&(resourceManager->m_memoryAllocator), true);
	m_pCamera->m_type = MyCamera::CameraType::lookat;
	//m_pCamera->SetPosition(glm::vec3(10, 10, 10));
	//m_pCamera->SetRotation(glm::vec3(-45, 0, 45));

	m_pCamera->SetPerspective(45.0f, (float)width / (float)height, 0.1f, 10.0f);

	m_pCamera->CreateDescriptorSet(m_pResourceManager->m_device, m_pResourceManager->m_descriptorPool);

	m_bbox.min = glm::vec3(INFINITE);
	m_bbox.max = -glm::vec3(INFINITE);

	m_pShadowMap = std::make_shared<ShadowMap>(m_pResourceManager, m_pPipelineManager, m_pRenderQueueManager);
}

RenderScene::~RenderScene()
{
}

void RenderScene::AddScene(std::shared_ptr<MyScene> scene)
{
	m_pShadowMap->AddScene(scene);

	auto drawables = scene->GetDrawables();

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
		id.model.materialPart.info.bits.metallicRoughnessMap = drawable->m_material->m_pMetallicRoughnessMap != nullptr;
		id.type = m_pipelineType;

		if (m_pRenderQueueManager->HasRenderQueue(id))
		{
			auto renderQueue = m_pRenderQueueManager->GetRenderQueue(id, m_framebuffers[0]->m_pRenderPass);
			renderQueue->AddDrawable(drawable);
		}
		else
		{
			auto renderQueue = m_pRenderQueueManager->GetRenderQueue(id, m_framebuffers[0]->m_pRenderPass);
			m_renderQueues.push_back(renderQueue);
		}
	}

	if (m_bbox.min.x > scene->m_bbox.min.x)
	{
		m_bbox.min.x = scene->m_bbox.min.x;
	}
	if (m_bbox.min.y > scene->m_bbox.min.y)
	{
		m_bbox.min.y = scene->m_bbox.min.y;
	}
	if (m_bbox.min.z > scene->m_bbox.min.z)
	{
		m_bbox.min.z = scene->m_bbox.min.z;
	}

	if (m_bbox.max.x < scene->m_bbox.max.x)
	{
		m_bbox.max.x = scene->m_bbox.max.x;
	}
	if (m_bbox.max.y < scene->m_bbox.max.y)
	{
		m_bbox.max.y = scene->m_bbox.max.y;
	}
	if (m_bbox.max.z < scene->m_bbox.max.z)
	{
		m_bbox.max.z = scene->m_bbox.max.z;
	}

	//glm::vec3 center = (m_bbox.max + m_bbox.min) * 0.5f;
	//glm::vec3 boxverts[8] = { glm::vec3(m_bbox.max.x, m_bbox.max.y, m_bbox.min.z), glm::vec3(m_bbox.max.x, m_bbox.min.y, m_bbox.min.z),
	//	glm::vec3(m_bbox.min.x, m_bbox.max.y, m_bbox.min.z), glm::vec3(m_bbox.min.x, m_bbox.min.y, m_bbox.min.z),
	//	glm::vec3(m_bbox.max.x, m_bbox.max.y, m_bbox.max.z), glm::vec3(m_bbox.max.x, m_bbox.min.y, m_bbox.max.z),
	//	glm::vec3(m_bbox.min.x, m_bbox.max.y, m_bbox.max.z), glm::vec3(m_bbox.min.x, m_bbox.min.y, m_bbox.max.z) };

	//float maxlength = 0;
	//glm::vec3 lightdir = glm::normalize(glm::vec3(-1, -1, -1));
	//for (int i = 0; i < 8; ++i)
	//{
	//	float temp = glm::dot(-lightdir, boxverts[i] - center);
	//	if (maxlength < temp)
	//	{
	//		maxlength = temp;
	//	}
	//}
	//glm::vec3 eye = maxlength * (-lightdir) + center;

	//m_pcamera->lookat(eye, center, glm::vec3(0, 0, 1));

	m_pCamera->SetRotation(glm::vec3(0, 0, 0));

	glm::vec3 center = (m_bbox.min + m_bbox.max) * 0.5f;
	float length = glm::length(m_bbox.max - m_bbox.min) * 0.5;
	glm::vec3 eye = length * glm::vec3(0, 0, 1) + center;

	m_pCamera->SetPosition(-eye);
	//m_pcamera->lookat(eye, center, glm::vec3(0, 0, 1));

	_updateBBox();
}

void RenderScene::Draw(vk::CommandBuffer& commandBuffer)
{
	_begin(commandBuffer);
	// drawAxis
	m_pAxis->CreateDrawCommand(commandBuffer, m_pCamera->m_descriptorSet, m_framebuffers[0]->m_pRenderPass);

	for (auto renderQueue : m_renderQueues)
	{
		renderQueue->Draw(commandBuffer, m_pCamera, m_pSkybox);
	}
	
	m_pSkybox->Draw(commandBuffer, m_pCamera, m_framebuffers[0]->m_pRenderPass);

	_end(commandBuffer);
}

void RenderScene::UpdateUniforms()
{
	_updateBBox();
	m_pCamera->UpdateUniformBuffer();
	m_pSkybox->m_pSHLight->UpdateUniformBuffer();
	if (m_pShadowMap)
	{
		m_pShadowMap->m_pCamera->UpdateUniformBuffer();
	}
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

void RenderScene::_updateBBox()
{
	float length = glm::length(m_bbox.max - m_bbox.min);
	float farPlane = glm::length(m_pCamera->m_position - (float)0.5 * (m_bbox.max + m_bbox.min)) + length / 2;
	m_pCamera->SetPerspective(45.0f, (float)m_width / (float)m_height, 0.1f, farPlane);
}
