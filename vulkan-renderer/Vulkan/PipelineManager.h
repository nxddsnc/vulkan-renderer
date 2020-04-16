#include "Pipeline.h"
#include <unordered_map>
#include <memory>
#include "Platform.h"

#pragma once

class VulkanRenderer;
class RenderPass;
class PipelineManager
{
public:
  PipelineManager(VulkanRenderer *vulkanRenderer);
  ~PipelineManager();

  std::shared_ptr<Pipeline> GetPipeline(PipelineId id, std::shared_ptr<RenderPass> renderPass); 

private:
  std::shared_ptr<Pipeline> _createPipeline(PipelineId id, std::shared_ptr<RenderPass> renderPass);
private:
  std::unordered_map<PipelineId, std::shared_ptr<Pipeline>> _pipelines;
  VulkanRenderer *_renderer;
};

