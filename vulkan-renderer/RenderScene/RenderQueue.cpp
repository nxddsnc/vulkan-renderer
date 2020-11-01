#include "RenderQueue.h"
#include "Drawable.h"
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

void RenderQueue::AddDrawable(std::shared_ptr<Drawable> drawable)
{
	drawable->m_pPipeline = m_pPipeline;
	m_drawables.push_back(drawable);
}

void RenderQueue::Draw(vk::CommandBuffer commandBuffer, std::shared_ptr<MyCamera> camera, std::shared_ptr<Skybox> skybox, std::shared_ptr<ShadowMap> shadowMap, int width, int height)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipeline());

	for (auto drawable : m_drawables)
	{
		uint32_t bufferNumber = m_pPipeline->m_id.type != DEPTH ? drawable->m_vertexBuffers.size() : 1;
		commandBuffer.bindVertexBuffers(0, drawable->m_vertexBuffers.size(), drawable->m_vertexBuffers.data(), drawable->m_vertexBufferOffsets.data());

		if (drawable->m_type == INSTANCE_DRAWABLE)
		{
			auto drawable_ = std::dynamic_pointer_cast<InstanceDrawable>(drawable);
			commandBuffer.bindVertexBuffers(drawable_->m_vertexBuffers.size(), drawable_->m_instanceBuffer.size(), drawable_->m_instanceBuffer.data(), drawable_->m_instanceBufferOffsets.data());
		}
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
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 0, 1, &camera->m_descriptorSet, 0, nullptr);
		//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 1, 1, &skybox->m_pSHLight->m_descriptorSet, 0, nullptr);
		//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), 2, 1, &skybox->m_preFilteredDescriptorSet, 0, nullptr);

        int bindings = 1;
		if (m_pPipeline->m_id.type != DEPTH && (drawable->baseColorTexture || drawable->normalTexture || drawable->metallicRoughnessTexture))
		{
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), bindings++, 1, &drawable->textureDescriptorSet, 0, nullptr);
		}

        if (drawable->m_pAnimation && m_pPipeline->m_id.type == MODEL_DEFERRED)
        {
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pPipeline->GetPipelineLayout(), bindings++, 1, &drawable->m_pAnimation->m_descriptorSet, 0, nullptr);
        }

		if (width != 0 && height != 0)
		{
			vk::Viewport viewport(0, 0, width, height, 0, 1);
			vk::Rect2D sissor(vk::Offset2D(0, 0), vk::Extent2D(width, height));
			commandBuffer.setViewport(0, 1, &viewport);
			commandBuffer.setScissor(0, 1, &sissor);
		}

		uint32_t offset = 0;
		if (drawable->m_type == SINGLE_DRAWABLE)
		{
			std::shared_ptr<SingleDrawable> drawable_ = std::dynamic_pointer_cast<SingleDrawable>(drawable);
			commandBuffer.pushConstants(m_pPipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, offset, sizeof(glm::mat4), reinterpret_cast<void*>(&drawable_->m_matrix));
			offset += sizeof(glm::mat4);
			commandBuffer.pushConstants(m_pPipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, offset, sizeof(glm::mat4), reinterpret_cast<void*>(&drawable_->m_normalMatrix));
			offset += sizeof(glm::mat4);
		}

		if (m_pPipeline->m_id.model.materialPart.info.bits.baseColorInfo)
		{
			commandBuffer.pushConstants(m_pPipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, offset, sizeof(glm::vec4), reinterpret_cast<void*>(&drawable->m_material->m_baseColor));
			offset += sizeof(glm::vec4);
		}
		if (m_pPipeline->m_id.model.materialPart.info.bits.metallicRoughnessInfo && m_pPipeline->m_id.type != DEPTH)
		{
			commandBuffer.pushConstants(m_pPipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, offset, sizeof(glm::vec2), reinterpret_cast<void*>(&drawable->m_material->m_metallicRoughness));
			offset += sizeof(glm::vec2);
		}

		if (drawable->m_type == INSTANCE_DRAWABLE)
		{
			std::shared_ptr<InstanceDrawable> instance = std::dynamic_pointer_cast<InstanceDrawable>(drawable);
			commandBuffer.drawIndexed(instance->m_mesh->m_indexNum, instance->m_matricies.size(), 0, 0, 0);
		}
		else 
		{
			commandBuffer.drawIndexed(drawable->m_mesh->m_indexNum, 1, 0, 0, 0);
		}
	}

}
