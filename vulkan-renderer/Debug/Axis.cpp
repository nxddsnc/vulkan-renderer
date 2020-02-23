#include "Axis.h"
#include "MyMesh.h"
#include "ResourceManager.h"
#include "Drawable.h"
#include "Pipeline.h"
#include "PipelineManager.h"

Axis::Axis(ResourceManager *pResourceManager, PipelineManager *pPipelineManager)
{
    m_pDrawable = std::make_shared<Drawable>();
    m_pDrawable->m_mesh = std::make_shared<MyMesh>();
    m_pDrawable->m_mesh->CreateAixs();

    pResourceManager->InitVulkanBuffers(m_pDrawable);

    m_pResouceManager = pResourceManager;
    m_pPipelineMananger = pPipelineManager;
}

Axis::~Axis()
{
    
}

void Axis::CreateDrawCommand(vk::CommandBuffer & commandBuffer, vk::DescriptorSet descriptorSet)
{
    PipelineId pipelineId;
    PipelineId linesPipelineId;
    linesPipelineId.type = PipelineType::MODEL;
    linesPipelineId.model.primitivePart.info.bits.primitiveMode = PrimitiveMode::Lines;
    linesPipelineId.model.primitivePart.info.bits.positionVertexData = 1;
    linesPipelineId.model.primitivePart.info.bits.normalVertexData = 0;
    linesPipelineId.model.primitivePart.info.bits.countTexCoord = 0;
    linesPipelineId.model.primitivePart.info.bits.tangentVertexData = 0;
    linesPipelineId.model.primitivePart.info.bits.countColor = 1;
    std::shared_ptr<Pipeline> pipelineLines = m_pPipelineMananger->GetPipeline(linesPipelineId);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineLines->GetPipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLines->GetPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
    commandBuffer.bindVertexBuffers(0, m_pDrawable->m_vertexBuffers.size(), m_pDrawable->m_vertexBuffers.data(), m_pDrawable->m_vertexBufferOffsets.data());
    commandBuffer.bindIndexBuffer(m_pDrawable->m_indexBuffer, 0, vk::IndexType::eUint16);

    commandBuffer.drawIndexed(m_pDrawable->m_mesh->m_indexNum, 1, 0, 0, 0);
}


