#include "Platform.h"
#include "Pipeline.h"
#include <vector>
#pragma once

enum ShaderStage
{
    VERTEX,
    GEOMETRY,
    TESSLATION,
    FRAGMENT,
};
class ShaderModule
{
public:
  ShaderModule(vk::Device *device, PipelineId id);
  ~ShaderModule();

  vk::ShaderModule GetShaderModule() { return _shaderModule; };
  vk::PipelineShaderStageCreateInfo GetShaderStageCreateInfo() { return _stageCreateInfo; }

  void BuildFromFile(const std::string& filename, ShaderStage stage, const char *entry);
private:
    PipelineId                              _id;
    vk::ShaderModule                        _shaderModule;
    vk::PipelineShaderStageCreateInfo       _stageCreateInfo;
    vk::Device                             *_device;
    std::vector<char>                       _data;
};