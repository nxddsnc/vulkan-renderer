#include "Platform.h"
#include "Pipeline.h"
#include <vector>
class ShaderModule
{
public:
  ShaderModule(PipelineId id);
  ~ShaderModule();

  vk::ShaderModule Get() { return _shaderModule; };
  vk::PipelineShaderStageCreateInfo GetShaderStageCreateInfo() { return _stageCreateInfo; }
private:
    vk::ShaderModule                        _shaderModule;
    vk::PipelineShaderStageCreateInfo       _stageCreateInfo;
    vk::Device                             *_device;
     
    std::vector<char>                       _data;
}