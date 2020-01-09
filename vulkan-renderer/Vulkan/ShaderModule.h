#include "Platform.h"
#include "Pipeline.h"
#include <vector>
class ShaderModule
{
public:
  ShaderModule(PipelineId id);
  ~ShaderModule();

  vk::ShaderModule Get() { return _shaderModule; };
private:
    vk::ShaderModule      _shaderModule;
    vk::Device           *_device;
     
    std::vector<char>     _data;
}