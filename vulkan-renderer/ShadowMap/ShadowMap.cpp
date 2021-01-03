#include "ShadowMap.h"
#include "Framebuffer.h"
#include "MyImage.h"
#include "Renderable.h"
#include "ResourceManager.h"
#include "Pipeline.h"
#include "PipelineManager.h"
#include "MyScene.h"
#include "RenderQueueManager.h"
#include "RenderQueue.h"
#include "RenderPass.h"
#include "MyCamera.h"

ShadowMap::ShadowMap(ResourceManager *resourceManager, PipelineManager *pipelineManager, std::shared_ptr<RenderQueueManager> renderQueueManager)
{
	m_pResourceManager = resourceManager;
	m_pPipelineManager = pipelineManager;
	m_pRenderQueuemanager = renderQueueManager;

	m_pCamera = std::make_shared<MyCamera>(&resourceManager->m_memoryAllocator, true);

	m_width = 2048;
	m_height = 2048;

	m_bbox.min = glm::vec3(INFINITE, INFINITE, INFINITE);
	m_bbox.max = -glm::vec3(INFINITE, INFINITE, INFINITE);
	
	m_lightDir = glm::normalize(glm::vec3(-1, -1, -1));

	_init();
}

ShadowMap::~ShadowMap()
{
	_deInit();
}

std::shared_ptr<Framebuffer> ShadowMap::GetFramebuffer()
{
	return m_pFramebuffer;
}

void ShadowMap::AddScene(std::shared_ptr<MyScene> scene)
{
	auto renderables = scene->GetRenderables();

	PipelineId id;

	id.model.primitivePart.info.bits.positionVertexData = 1;
	id.model.primitivePart.info.bits.normalVertexData = 1;
	id.model.primitivePart.info.bits.primitiveMode = PrimitiveMode::Triangles;
	id.model.materialPart.info.bits.baseColorInfo = 0;
	id.model.materialPart.info.bits.baseColorMap = 0;
	id.model.materialPart.info.bits.normalMap = 0;
	id.model.materialPart.info.bits.metallicRoughnessMap = 0;
	id.type = DEPTH;
	for (int i = 0; i < renderables.size(); ++i)
	{
		std::shared_ptr<Renderable> renderable = renderables[i];

		id.model.primitivePart.info.bits.tangentVertexData = renderable->m_mesh->m_vertexBits.hasTangent;
		id.model.primitivePart.info.bits.countTexCoord = renderable->m_mesh->m_vertexBits.hasTexCoord0 ? 1 : 0;
		id.model.primitivePart.info.bits.countColor = renderable->m_mesh->m_vertexBits.hasColor;
		id.model.primitivePart.info.bits.jointVertexData = renderable->m_mesh->m_vertexBits.hasBone;
		id.model.primitivePart.info.bits.weightVertexData = renderable->m_mesh->m_vertexBits.hasBone;
		if (renderable->m_type == INSTANCE_RENDERABLE)
		{
			id.model.primitivePart.info.bits.instanceMatrixData = 1;
		}
		else
		{
			id.model.primitivePart.info.bits.instanceMatrixData = 0;
		}

		m_pResourceManager->InitVulkanResource(renderable);

		if (m_pRenderQueuemanager->HasRenderQueue(id))
		{
			auto renderQueue = m_pRenderQueuemanager->GetRenderQueue(id, m_pFramebuffer->m_pRenderPass);
			renderQueue->AddRenderable(renderable);
		} 
		else
		{
			auto renderQueue = m_pRenderQueuemanager->GetRenderQueue(id, m_pFramebuffer->m_pRenderPass);
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

	glm::vec3 center = (m_bbox.max + m_bbox.min) * 0.5f;
	glm::vec3 boxVerts[8] = { glm::vec3(m_bbox.max.x, m_bbox.max.y, m_bbox.min.z), glm::vec3(m_bbox.max.x, m_bbox.min.y, m_bbox.min.z),
		glm::vec3(m_bbox.min.x, m_bbox.max.y, m_bbox.min.z), glm::vec3(m_bbox.min.x, m_bbox.min.y, m_bbox.min.z),
		glm::vec3(m_bbox.max.x, m_bbox.max.y, m_bbox.max.z), glm::vec3(m_bbox.max.x, m_bbox.min.y, m_bbox.max.z),
		glm::vec3(m_bbox.min.x, m_bbox.max.y, m_bbox.max.z), glm::vec3(m_bbox.min.x, m_bbox.min.y, m_bbox.max.z) };

	m_pCamera->UpdateBBox(m_bbox);
	m_pCamera->m_alphaTarget = 45.0 / 180.0 * 3.14;
	m_pCamera->m_betaTarget = 45.0 / 180.0 * 3.14;
}

std::shared_ptr<Framebuffer> ShadowMap::Draw(vk::CommandBuffer& commandBuffer)
{
	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].color.float32[0] = 1.0;
	clearValues[0].color.float32[1] = 0.0;
	clearValues[0].color.float32[2] = 0.0;
	clearValues[0].color.float32[3] = 1.0f;

	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	vk::RenderPassBeginInfo renderPassInfo(m_pFramebuffer->m_pRenderPass->Get(), m_pFramebuffer->m_vkFramebuffer,
		vk::Rect2D(vk::Offset2D(0, 0),
			vk::Extent2D(m_width, m_height)),
		(uint32_t)clearValues.size(),
		clearValues.data());

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

	for (auto renderQueue : m_renderQueues)
	{
		renderQueue->Draw(commandBuffer, m_pCamera, nullptr, nullptr, m_width, m_height);
	}

	commandBuffer.endRenderPass();
	return m_pFramebuffer;
}

void ShadowMap::UpdateUniform()
{
    m_pCamera->Update(m_width, m_height);
}

void ShadowMap::_init()
{
	std::vector<MyImageFormat> formats = {  };
	m_pCamera = std::make_shared<MyCamera>(&m_pResourceManager->m_memoryAllocator, true);

	m_pCamera->CreateDescriptorSet(m_pResourceManager->m_device, m_pResourceManager->m_descriptorPool);
	m_pFramebuffer = std::make_shared<Framebuffer>("shadow-map", m_pResourceManager, formats, MY_IMAGEFORMAT_D32_FLOAT, m_width, m_height, true);
}

void ShadowMap::_deInit()
{

}

void ShadowMap::_update()
{
}
