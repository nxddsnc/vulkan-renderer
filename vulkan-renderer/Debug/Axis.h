#pragma once
#include <memory>
#include "Platform.h"

class MyMesh;
class SingleRenderable;
class ResourceManager;
class PipelineManager;
class RenderPass;
class Axis
{
public:
    Axis(ResourceManager *pResourceManager, PipelineManager *pPipelineManager);
    ~Axis();

    void CreateDrawCommand(vk::CommandBuffer &commandBuffer, vk::DescriptorSet descriptorSet, std::shared_ptr<RenderPass> renderPass);

    std::shared_ptr<SingleRenderable> m_pRenderable;

    ResourceManager * m_pResouceManager;
    PipelineManager * m_pPipelineMananger;
};