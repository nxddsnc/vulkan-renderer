#include "PipelineManager.h"

PipelineManager::PipelineManager(VulkanRenderer *vulkanRenderer)
{
  _renderer =  vulkanRenderer;
}

PipelineManager::~PipelineManager()
{
    for (auto it : _pipelines)
    {
        it.second->Destroy();
    }
}

std::shared_ptr<Pipeline> PipelineManager::_createPipeline(PipelineId id, std::shared_ptr<RenderPass> renderPass)
{
  std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(_renderer, id);

  switch(id.type)
  {
  case MODEL_FORWARD:
      pipeline->InitModel(renderPass);
	  pipeline->m_bReady = true;
      break;
  case SKYBOX:
      pipeline->InitSkybox(renderPass);
	  pipeline->m_bReady = true;
	  break;
  //case PREFILTERED_CUBE_MAP:
  //    pipeline->InitPrefilteredCubeMap();
  }
  
  return pipeline;
}

std::shared_ptr<Pipeline> PipelineManager::GetPipeline(PipelineId id, std::shared_ptr<RenderPass> renderPass)
{
  if (_pipelines.count(id) > 0) 
  {
    return _pipelines.at(id);
  }
  else 
  {
  auto pipeline = _createPipeline(id, renderPass);
    _pipelines.insert(std::make_pair(id, pipeline));
    return pipeline;
  }
}