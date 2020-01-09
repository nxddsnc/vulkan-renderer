#inlcude "ShaderModule.h"
#include <ifstream>

ShaderModule(PipelineId id) 
{

}

~ShaderModule() 
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

bool ShaderModule::build(API::ShaderModule& shaderModule, VkResult* returnResult) {
  vk::ShaderModuleCreateInfo createInfo = {};
	createInfo.codeSize = _data.size();
	createInfo.pCode = _data.data();
	VkShaderModule shaderModule;
	vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule);
	return shaderModule;
}