#pragma once
#include <memory>
#include "Platform.h"

class MyMesh;
class Drawable;
class ResourceManager;
class PipelineManager;
class Axis
{
public:
    Axis(ResourceManager *pResourceManager, PipelineManager *pPipelineManager);
    ~Axis();

    void CreateDrawCommand(vk::CommandBuffer &commandBuffer);

    std::shared_ptr<Drawable> m_pDrawable;

    ResourceManager * m_pResouceManager;
    PipelineManager * m_pPipelineMananger;
};