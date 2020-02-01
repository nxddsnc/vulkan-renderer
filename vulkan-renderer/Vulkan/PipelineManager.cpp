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

std::shared_ptr<Pipeline> PipelineManager::_createPipeline(PipelineId id)
{
  std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(_renderer, id);

  switch(id.type)
  {
  case MODEL:
      pipeline->InitModel();
      break;
  case SKYBOX:
      pipeline->InitSkybox();
      break;
  }
  
  return pipeline;
}

std::shared_ptr<Pipeline> PipelineManager::GetPipeline(PipelineId id)
{
  if (_pipelines.count(id) > 0) 
  {
    return _pipelines.at(id);
  }
  else 
  {
  auto pipeline = _createPipeline(id);
    _pipelines.insert(std::make_pair(id, pipeline));
    return pipeline;
  }
}