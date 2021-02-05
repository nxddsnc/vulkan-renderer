#include "Renderable.h"
#include "MyCamera.h"
#include "Pipeline.h"
#include <math.h>
#include "vulkan.hpp"


SingleRenderable::SingleRenderable()
{
	m_bbox.min = glm::vec3(INFINITE);
	m_bbox.max = -glm::vec3(INFINITE);
	m_type = SINGLE_RENDERABLE;
	m_bReady = false;
}

SingleRenderable::~SingleRenderable()
{
}

void SingleRenderable::ComputeBBox()
{
	BBox box = {glm::vec3(INFINITE), -glm::vec3(INFINITE)};

	for (int i = 0; i < m_mesh->m_vertexNum; ++i)
	{
		if (box.min.x > m_mesh->m_positions[i].x)
		{
			box.min.x = m_mesh->m_positions[i].x;
		}
		if (box.min.y > m_mesh->m_positions[i].y)
		{
			box.min.y = m_mesh->m_positions[i].y;
		}
		if (box.min.z > m_mesh->m_positions[i].z)
		{
			box.min.z = m_mesh->m_positions[i].z;
		}

		if (box.max.x < m_mesh->m_positions[i].x)
		{
			box.max.x = m_mesh->m_positions[i].x;
		}
		if (box.max.y < m_mesh->m_positions[i].y)
		{
			box.max.y = m_mesh->m_positions[i].y;
		}
		if (box.max.z < m_mesh->m_positions[i].z)
		{
			box.max.z = m_mesh->m_positions[i].z;
		}
	}

	glm::vec3 boxVerts[8] = { glm::vec3(box.max.x, box.max.y, box.min.z), glm::vec3(box.max.x, box.min.y, box.min.z), 
		glm::vec3(box.min.x, box.max.y, box.min.z), glm::vec3(box.min.x, box.min.y, box.min.z),
		glm::vec3(box.max.x, box.max.y, box.max.z), glm::vec3(box.max.x, box.min.y, box.max.z),
		glm::vec3(box.min.x, box.max.y, box.max.z), glm::vec3(box.min.x, box.min.y, box.max.z) };

	for (int i = 0; i < 8; ++i)
	{
		glm::vec4 vec = m_matrix * glm::vec4(boxVerts[i].x, boxVerts[i].y, boxVerts[i].z, 1);
		if (m_bbox.min.x > vec.x)
		{
			m_bbox.min.x = vec.x;
		}
		if (m_bbox.min.y > vec.y)
		{
			m_bbox.min.y = vec.y;
		}
		if (m_bbox.min.z > vec.z)
		{
			m_bbox.min.z = vec.z;
		}

		if (m_bbox.max.x < vec.x)
		{
			m_bbox.max.x = vec.x;
		}
		if (m_bbox.max.y < vec.y)
		{
			m_bbox.max.y = vec.y;
		}
		if (m_bbox.max.z < vec.z)
		{
			m_bbox.max.z = vec.z;
		}
	}
}

void SingleRenderable::Render(vk::CommandBuffer & commandBuffer, Pipeline* pipeline, MyCamera* camera)
{
	uint32_t bufferNumber = pipeline->m_id.type != DEPTH ? m_vertexBuffers.size() : 1;
	commandBuffer.bindVertexBuffers(0, bufferNumber, m_vertexBuffers.data(), m_vertexBufferOffsets.data());

	switch (m_mesh->m_indexType)
	{
	case 2:
		commandBuffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint16);
		break;
	case 4:
		commandBuffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint32);
		break;
	}

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(), 0, 1, &camera->m_descriptorSet, 0, nullptr);

	int bindings = 1;
	if (pipeline->m_id.type != DEPTH && (baseColorTexture || normalTexture || metallicRoughnessTexture))
	{
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(), bindings++, 1, &textureDescriptorSet, 0, nullptr);
	}

	//if (m_pAnimation && pipeline->m_id.type == MODEL_DEFERRED)
	//{
	//	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(), bindings++, 1, &m_pAnimation->m_descriptorSet, 0, nullptr);
	//}

	uint32_t offset = 0;

	commandBuffer.pushConstants(pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, offset, sizeof(glm::mat4), reinterpret_cast<void*>(&m_matrix));
	offset += sizeof(glm::mat4);
	commandBuffer.pushConstants(pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, offset, sizeof(glm::mat4), reinterpret_cast<void*>(&m_normalMatrix));
	offset += sizeof(glm::mat4);

	if (pipeline->m_id.model.materialPart.info.bits.baseColorInfo)
	{
		commandBuffer.pushConstants(pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, offset, sizeof(glm::vec4), reinterpret_cast<void*>(&m_material->m_baseColor));
		offset += sizeof(glm::vec4);
	}
	if (pipeline->m_id.model.materialPart.info.bits.metallicRoughnessInfo && pipeline->m_id.type != DEPTH)
	{
		commandBuffer.pushConstants(pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, offset, sizeof(glm::vec2), reinterpret_cast<void*>(&m_material->m_metallicRoughness));
		offset += sizeof(glm::vec2);
	}

	commandBuffer.drawIndexed(m_mesh->m_indexNum, 1, 0, 0, 0);
}

// TODO: Not efficient way to initialize variables.
InstanceRenderable::InstanceRenderable()
{
	m_bbox.min = glm::vec3(INFINITE);
	m_bbox.max = -glm::vec3(INFINITE);
	m_type = INSTANCE_RENDERABLE;
	m_bReady = false;
}

InstanceRenderable::InstanceRenderable(std::shared_ptr<SingleRenderable> d)
{
	m_mesh = d->m_mesh;
	m_material = d->m_material;
	m_type = INSTANCE_RENDERABLE;
	m_matricies.push_back(d->m_matrix);
	m_normalMatrices.push_back(d->m_normalMatrix);
	m_bReady = false;
	m_matrixCols.emplace_back(std::vector<glm::vec4>());
	m_matrixCols.emplace_back(std::vector<glm::vec4>());
	m_matrixCols.emplace_back(std::vector<glm::vec4>());
	m_matrixCols[0].emplace_back(glm::vec4(d->m_matrix[0][0], d->m_matrix[1][0], d->m_matrix[2][0], d->m_matrix[3][0]));
	m_matrixCols[1].emplace_back(glm::vec4(d->m_matrix[0][1], d->m_matrix[1][1], d->m_matrix[2][1], d->m_matrix[3][1]));
	m_matrixCols[2].emplace_back(glm::vec4(d->m_matrix[0][2], d->m_matrix[1][2], d->m_matrix[2][2], d->m_matrix[3][2]));
}

InstanceRenderable::~InstanceRenderable()
{
}

void InstanceRenderable::AddRenderable(std::shared_ptr<SingleRenderable> d)
{
	m_matricies.push_back(d->m_matrix);
	// TODO: not cache friendly.
	m_matrixCols[0].push_back(glm::vec4(d->m_matrix[0][0], d->m_matrix[1][0], d->m_matrix[2][0], d->m_matrix[3][0]));
	m_matrixCols[1].push_back(glm::vec4(d->m_matrix[0][1], d->m_matrix[1][1], d->m_matrix[2][1], d->m_matrix[3][1]));
	m_matrixCols[2].push_back(glm::vec4(d->m_matrix[0][2], d->m_matrix[1][2], d->m_matrix[2][2], d->m_matrix[3][2]));
	m_normalMatrices.push_back(d->m_normalMatrix);
}

void InstanceRenderable::ComputeBBox()
{
	BBox box = { glm::vec3(INFINITE), -glm::vec3(INFINITE) };

	for (int i = 0; i < m_mesh->m_vertexNum; ++i)
	{
		if (box.min.x > m_mesh->m_positions[i].x)
		{
			box.min.x = m_mesh->m_positions[i].x;
		}
		if (box.min.y > m_mesh->m_positions[i].y)
		{
			box.min.y = m_mesh->m_positions[i].y;
		}
		if (box.min.z > m_mesh->m_positions[i].z)
		{
			box.min.z = m_mesh->m_positions[i].z;
		}

		if (box.max.x < m_mesh->m_positions[i].x)
		{
			box.max.x = m_mesh->m_positions[i].x;
		}
		if (box.max.y < m_mesh->m_positions[i].y)
		{
			box.max.y = m_mesh->m_positions[i].y;
		}
		if (box.max.z < m_mesh->m_positions[i].z)
		{
			box.max.z = m_mesh->m_positions[i].z;
		}
	}

	glm::vec3 boxVerts[8] = { glm::vec3(box.max.x, box.max.y, box.min.z), glm::vec3(box.max.x, box.min.y, box.min.z),
		glm::vec3(box.min.x, box.max.y, box.min.z), glm::vec3(box.min.x, box.min.y, box.min.z),
		glm::vec3(box.max.x, box.max.y, box.max.z), glm::vec3(box.max.x, box.min.y, box.max.z),
		glm::vec3(box.min.x, box.max.y, box.max.z), glm::vec3(box.min.x, box.min.y, box.max.z) };

	for (int i = 0; i < m_matricies.size(); ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			glm::vec4 vec = m_matricies[i] * glm::vec4(boxVerts[j].x, boxVerts[j].y, boxVerts[j].z, 1);
			if (m_bbox.min.x > vec.x)
			{
				m_bbox.min.x = vec.x;
			}
			if (m_bbox.min.y > vec.y)
			{
				m_bbox.min.y = vec.y;
			}
			if (m_bbox.min.z > vec.z)
			{
				m_bbox.min.z = vec.z;
			}

			if (m_bbox.max.x < vec.x)
			{
				m_bbox.max.x = vec.x;
			}
			if (m_bbox.max.y < vec.y)
			{
				m_bbox.max.y = vec.y;
			}
			if (m_bbox.max.z < vec.z)
			{
				m_bbox.max.z = vec.z;
			}
		}

	}
}

void InstanceRenderable::Render(vk::CommandBuffer & commandBuffer, Pipeline* pipeline, MyCamera* camera)
{
	uint32_t bufferNumber = pipeline->m_id.type != DEPTH ? m_vertexBuffers.size() : 1;
	commandBuffer.bindVertexBuffers(0, bufferNumber, m_vertexBuffers.data(), m_vertexBufferOffsets.data());
	commandBuffer.bindVertexBuffers(bufferNumber, m_instanceBuffer.size(), m_instanceBuffer.data(), m_instanceBufferOffsets.data());
	
	switch (m_mesh->m_indexType)
	{
	case 2:
		commandBuffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint16);
		break;
	case 4:
		commandBuffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint32);
		break;
	}

	//pipelineBindPoint, PipelineLayout, firstSet, descriptorSetCount, pDescriptorSets, uint32_t dynamicOffsetCount.
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(), 0, 1, &camera->m_descriptorSet, 0, nullptr);
	//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(), 1, 1, &skybox->m_pSHLight->m_descriptorSet, 0, nullptr);
	//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(), 2, 1, &skybox->m_preFilteredDescriptorSet, 0, nullptr);

	int bindings = 1;
	if (pipeline->m_id.type != DEPTH && (baseColorTexture || normalTexture || metallicRoughnessTexture))
	{
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(), bindings++, 1, &textureDescriptorSet, 0, nullptr);
	}

	//if (m_pAnimation && pipeline->m_id.type == MODEL_DEFERRED)
	//{
	//	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetPipelineLayout(), bindings++, 1, &m_pAnimation->m_descriptorSet, 0, nullptr);
	//}

	uint32_t offset = 0;

	if (pipeline->m_id.model.materialPart.info.bits.baseColorInfo)
	{
		commandBuffer.pushConstants(pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, offset, sizeof(glm::vec4), reinterpret_cast<void*>(&m_material->m_baseColor));
		offset += sizeof(glm::vec4);
	}
	if (pipeline->m_id.model.materialPart.info.bits.metallicRoughnessInfo && pipeline->m_id.type != DEPTH)
	{
		commandBuffer.pushConstants(pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, offset, sizeof(glm::vec2), reinterpret_cast<void*>(&m_material->m_metallicRoughness));
		offset += sizeof(glm::vec2);
	}

	commandBuffer.drawIndexed(m_mesh->m_indexNum, m_matricies.size(), 0, 0, 0);
}

std::size_t Renderable::GetHash()
{
	std::size_t res = 0;
	hash_combine(res, m_mesh.get());
	hash_combine(res, m_material.get());
    return res;
}
