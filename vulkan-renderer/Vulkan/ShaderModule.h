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

namespace shaderc
{
    struct CompileOptions;
}
class ShaderModule
{
public:
    ShaderModule(vk::Device *device, PipelineId id);
    ~ShaderModule();

    vk::ShaderModule GetShaderModule() { return _shaderModule; };
    vk::PipelineShaderStageCreateInfo GetShaderStageCreateInfo() { return _stageCreateInfo; }

    void BuildFromFile(const std::string& filename, ShaderStage stage, const char *entry, std::vector<std::string> macros = std::vector<std::string>());

private:
    void addShaderOptions(ShaderStage stage, shaderc::CompileOptions *option);
private:
    PipelineId                              _id;
    vk::ShaderModule                        _shaderModule;
    vk::PipelineShaderStageCreateInfo       _stageCreateInfo;
    vk::Device                             *_device;
    std::vector<char>                       _data;
};