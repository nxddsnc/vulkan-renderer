#include "Platform.h"
#include "Pipeline.h"
#include <vector>
#pragma once

class ShaderModule
{
public:
  ShaderModule(vk::Device *device, PipelineId id);
  ~ShaderModule();

  vk::ShaderModule Get() { return _shaderModule; };
  vk::PipelineShaderStageCreateInfo GetShaderStageCreateInfo() { return _stageCreateInfo; }

  void LoadFromFile(const std::string& filename);
  void Build(vk::ShaderStageFlagBits stage, const char *entry);
private:
    PipelineId                              _id;
    vk::ShaderModule                        _shaderModule;
    vk::PipelineShaderStageCreateInfo       _stageCreateInfo;
    vk::Device                             *_device;
    std::vector<char>                       _data;
};