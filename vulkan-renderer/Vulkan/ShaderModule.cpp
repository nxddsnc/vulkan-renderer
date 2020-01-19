#include "ShaderModule.h"
#include <fstream>
#include "shaderc\shaderc.hpp"

ShaderModule::ShaderModule(vk::Device* device, PipelineId id)
{
    _id = id;
    _device = device;
}

ShaderModule::~ShaderModule()
{
    _device->destroyShaderModule(_shaderModule);
}

void ShaderModule::BuildFromFile(const std::string& filename, ShaderStage stage, const char *entry)
{
  std::ifstream file(filename);

  if (!file.good())
  {
    throw std::runtime_error("failed to open file!");
  }
  std::string content = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

  file.close();

  shaderc::Compiler compiler;
  shaderc::CompileOptions options;

  shaderc_shader_kind kind;
  vk::ShaderStageFlagBits shaderStage;
  switch (stage)
  {
  case VERTEX:
      kind = shaderc_glsl_vertex_shader;
      shaderStage = vk::ShaderStageFlagBits::eVertex;
      break;
  case GEOMETRY:
      kind = shaderc_glsl_geometry_shader;
      shaderStage = vk::ShaderStageFlagBits::eGeometry;
      break;
  case TESSLATION:
      kind = shaderc_glsl_tess_control_shader;
      shaderStage = vk::ShaderStageFlagBits::eTessellationControl;
      break;
  case FRAGMENT:
      kind = shaderc_glsl_fragment_shader;
      shaderStage = vk::ShaderStageFlagBits::eFragment;
      break;
  default:
      break;
  }

  shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(content, kind, filename.c_str(), options);

  std::vector<uint32_t> data(module.cbegin(), module.cend());

  vk::ShaderModuleCreateInfo createInfo = {};
  createInfo.codeSize = data.size() * sizeof(uint32_t);
  createInfo.pCode = data.data();
  _shaderModule = _device->createShaderModule(createInfo);

  _stageCreateInfo.stage = shaderStage;
  _stageCreateInfo.module = _shaderModule;
  _stageCreateInfo.pName = entry;


      //Pipeline::Id::Model::PrimitivePart primitivePart = id.getModelPrimitivePart();
      //Pipeline::Id::Model::MaterialPart materialPart = id.getModelMaterialPart();
      //Pipeline::Id::Model::ExtraPart extraPart = id.getModelExtraPart();


      //// Primitive part
      //    options.AddMacroDefinition("IN_POSITION", std::to_string(primitivePart.positionVertexData));
      //    options.AddMacroDefinition("IN_NORMAL", std::to_string(primitivePart.normalVertexData));
      //    options.AddMacroDefinition("IN_TANGENT", std::to_string(primitivePart.tangentVertexData));
      //    options.AddMacroDefinition("IN_UV", std::to_string(primitivePart.countTexCoord));
      //    options.AddMacroDefinition("IN_COLOR", std::to_string(primitivePart.countColor));

      //// Material Part
      //    options.AddMacroDefinition("TEXTURE_COLOR", materialPart.baseColorInfo != 0b11 ? "1" : "0");
      //    options.AddMacroDefinition("TEXTURE_COLOR_UV", "inUV" + std::to_string(materialPart.baseColorInfo));

      //    options.AddMacroDefinition("TEXTURE_METALLIC_ROUGHNESS", materialPart.metallicRoughnessInfo != 0b11 ? "1" : "0");
      //    options.AddMacroDefinition("TEXTURE_METALLIC_ROUGHNESS_UV", "inUV" + std::to_string(materialPart.metallicRoughnessInfo));

      //    options.AddMacroDefinition("TEXTURE_NORMAL", materialPart.normalInfo != 0b11 ? "1" : "0");
      //    options.AddMacroDefinition("TEXTURE_NORMAL_UV", "inUV" + std::to_string(materialPart.normalInfo));

      //    options.AddMacroDefinition("TEXTURE_OCCLUSION", materialPart.occlusionInfo != 0b11 ? "1" : "0");
      //    options.AddMacroDefinition("TEXTURE_OCCLUSION_UV", "inUV" + std::to_string(materialPart.occlusionInfo));

      //    options.AddMacroDefinition("TEXTURE_EMISSIVE", materialPart.emissiveInfo != 0b11 ? "1" : "0");
      //    options.AddMacroDefinition("TEXTURE_EMISSIVE_UV", "inUV" + std::to_string(materialPart.emissiveInfo));
      //}

      //    options.AddMacroDefinition("TEXTURE_IRRADIANCE_MAP", extraPart.irradianceMapInfo ? "1" : "0");
      //    options.AddMacroDefinition("TEXTURE_PREFILTERED_MAP", extraPart.prefilteredMapInfo ? "1" : "0");

      //// Set location
      //{
      //    uint8_t location = 2;

      //    if (primitivePart.tangentVertexData) {
      //        options.AddMacroDefinition("IN_TANGENT_LOCATION", std::to_string(location++));
      //    }

      //    for (uint8_t i = 0; i < primitivePart.countTexCoord; ++i) {
      //        options.AddMacroDefinition("IN_UV_" + std::to_string(i) + "_LOCATION", std::to_string(location++));
      //    }

      //    for (uint8_t i = 0; i < primitivePart.countColor; ++i) {
      //        options.AddMacroDefinition("IN_COLOR_" + std::to_string(i) + "_LOCATION", std::to_string(location++));
      //    }

      //    options.AddMacroDefinition("IN_FREE_LOCATION", std::to_string(location++));
      //}

      //// Set binding
      //{
      //    uint8_t binding = 0;

      //    if (materialPart.baseColorInfo != 0b11) {
      //        options.AddMacroDefinition("TEXTURE_COLOR_BINDING", std::to_string(binding++));
      //    }

      //    if (materialPart.metallicRoughnessInfo != 0b11) {
      //        options.AddMacroDefinition("TEXTURE_METALLIC_ROUGHNESS_BINDING", std::to_string(binding++));
      //    }

      //    if (materialPart.normalInfo != 0b11) {
      //        options.AddMacroDefinition("TEXTURE_NORMAL_BINDING", std::to_string(binding++));
      //    }

      //    if (materialPart.occlusionInfo != 0b11) {
      //        options.AddMacroDefinition("TEXTURE_OCCLUSION_BINDING", std::to_string(binding++));
      //    }

      //    if (materialPart.emissiveInfo != 0b11) {
      //        options.AddMacroDefinition("TEXTURE_EMISSIVE_BINDING", std::to_string(binding++));
      //    }

      //    if (extraPart.irradianceMapInfo) {
      //        options.AddMacroDefinition("TEXTURE_IRRADIANCE_MAP_BINDING", std::to_string(binding++));
      //    }

      //    if (extraPart.prefilteredMapInfo) {
      //        options.AddMacroDefinition("TEXTURE_BRDF_LUT_BINDING", std::to_string(binding++));
      //        options.AddMacroDefinition("TEXTURE_PREFILTERED_MAP_BINDING", std::to_string(binding++));
      //    }
      //}
}
