#include "RenderQueue.h"
#include "Drawable.h"
#include "Pipeline.h"
#include "Camera.hpp"
#include "Skybox.h"
#include "SHLight.h"

RenderQueue::RenderQueue(std::shared_ptr<Pipeline> pipeline)
{
	m_pPipeline = pipeline;
}


RenderQueue::~RenderQueue()
{
}

void RenderQueue::AddDrawable(std::shared_ptr<Drawable> drawable)
{
	drawable->m_pPipeline = m_pPipeline;
	m_drawables.push_back(drawable);
}

void RenderQueue::Draw(vk::CommandBuffer commandBuffer, std::shared_ptr<VulkanCamera> camera, std::shared_ptr<Skybox> skybox)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipeline());

	for (auto drawable : m_drawables)
	{
		commandBuffer.bindVertexBuffers(0, drawable->m_vertexBuffers.size(), drawable->m_vertexBuffers.data(), drawable->m_vertexBufferOffsets.data());

		switch (drawable->m_mesh->m_indexType)
		{
		case 2:
			commandBuffer.bindIndexBuffer(drawable->m_indexBuffer, 0, vk::IndexType::eUint16);
			break;
		case 4:
			commandBuffer.bindIndexBuffer(drawable->m_indexBuffer, 0, vk::IndexType::eUint32);
			break;
		}

		//pipelineBindPoint, PipelineLayout, firstSet, descriptorSetCount, pDescriptorSets, uint32_t dynamicOffsetCount.
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 0, 1, &camera->descriptorSet, 0, nullptr);
		//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 1, 1, &skybox->m_pSHLight->m_descriptorSet, 0, nullptr);
		//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 2, 1, &skybox->m_preFilteredDescriptorSet, 0, nullptr);

		if (drawable->baseColorTexture || drawable->normalTexture || drawable->metallicRoughnessTexture)
		{
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 1, 1, &drawable->textureDescriptorSet, 0, nullptr);
		}

		uint32_t offset = 0;
		commandBuffer.pushConstants(m_pPipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, offset, sizeof(glm::mat4), reinterpret_cast<void*>(&drawable->m_matrix));
		offset += sizeof(glm::mat4);
		commandBuffer.pushConstants(m_pPipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, offset, sizeof(glm::mat4), reinterpret_cast<void*>(&drawable->m_normalMatrix));
		offset += sizeof(glm::mat4);

		if (m_pPipeline->m_id.model.materialPart.info.bits.baseColorInfo)
		{
			commandBuffer.pushConstants(m_pPipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, offset, sizeof(glm::vec4), reinterpret_cast<void*>(&drawable->m_material->m_baseColor));
			offset += sizeof(glm::vec4);
		}
		if (m_pPipeline->m_id.model.materialPart.info.bits.metallicRoughnessInfo)
		{
			commandBuffer.pushConstants(m_pPipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, offset, sizeof(glm::vec2), reinterpret_cast<void*>(&drawable->m_material->m_metallicRoughness));
			offset += sizeof(glm::vec2);
		}

		commandBuffer.drawIndexed(drawable->m_mesh->m_indexNum, 1, 0, 0, 0);
	}

}
