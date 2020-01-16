#include "ShaderModule.h"
#include <fstream>

ShaderModule::ShaderModule(vk::Device* device, PipelineId id)
{
    _id = id;
    _device = device;
}

ShaderModule::~ShaderModule()
{

}

void ShaderModule::LoadFromFile(const std::string& filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t)file.tellg();
  _data.resize(fileSize);

  file.seekg(0);
  file.read(_data.data(), fileSize);
  file.close();
}

void ShaderModule::Build() 
{
  vk::ShaderModuleCreateInfo createInfo = {};
	createInfo.codeSize = _data.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(_data.data());
    _shaderModule = _device->createShaderModule(&createInfo, nullptr);
}