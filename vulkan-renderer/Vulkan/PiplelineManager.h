#include "Pipeline.h"
#include <unordered_map>
#include <memory>
#include "Platform.h"


class VulkanRenderer;
class PiplelineManager
{
public:
  PiplelineManager(VulkanRenderer *vulkanRenderer);
  ~PiplelineManager();

  std::shared_ptr<Pipeline> GetPipeline(PipelineId id); 

private:
  std::shared_ptr<Pipeline> _createPipeline(PipelineId id);
private:
  std::unordered_map<PipelineId, std::shared_ptr<Pipeline>> _pipelines;
  VulkanRenderer _renderer;
};

