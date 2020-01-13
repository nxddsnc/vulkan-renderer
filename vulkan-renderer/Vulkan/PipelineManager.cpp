#include "PiplelineManager.h"

PiplelineManager::PiplelineManager(VulkanRenderer *vulkanRenderer)
{
  _renderer =  vulkanRenderer;
}

PiplelineManager::~PiplelineManager()
{
  
}

std::shared_ptr<Pipeline> PiplelineManager::_createPipeline(PipelineId id)
{
  std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(id);

  switch(id.type)
  {
    case MODEL:
      pipeline.InitModel();
      break;
  }


  
  return pipeline;
}

std::shared_ptr<Pipeline> PiplelineManager::GetPipeline(PipelineId id)
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