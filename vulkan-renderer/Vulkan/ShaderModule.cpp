#include "ShaderModule.h"
#include <fstream>
#include "shaderc\shaderc.hpp"
#include <iostream>

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
	addShaderOptions(stage, &options);

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

	if (module.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success)
	{
		std::cout << "Error when compiling shader: " << filename << std::endl;
		std::cout << module.GetErrorMessage() << std::endl;
		return;
	}

	vk::ShaderModuleCreateInfo createInfo = {};
	createInfo.codeSize = data.size() * sizeof(uint32_t);
	createInfo.pCode = data.data();
	_shaderModule = _device->createShaderModule(createInfo);

	_stageCreateInfo.stage = shaderStage;
	_stageCreateInfo.module = _shaderModule;
	_stageCreateInfo.pName = entry;
}

void ShaderModule::addShaderOptions(ShaderStage stage, shaderc::CompileOptions * options)
{
	uint8_t bindings = 0;
	switch (_id.type)
	{
	case PipelineType::MODEL_FORWARD:
	case PipelineType::MODEL_DEFERRED:
		bindings = 1;
		if (_id.model.primitivePart.info.bits.normalVertexData)
		{
			options->AddMacroDefinition("IN_NORMAL", "1");
			options->AddMacroDefinition("IN_NORMAL_LOCATION", std::to_string(bindings++));
		}
		if (_id.model.primitivePart.info.bits.countTexCoord)
		{
			options->AddMacroDefinition("IN_UV0", "1");
			options->AddMacroDefinition("IN_UV0_LOCATION", std::to_string(bindings++));
		}
		if (_id.model.primitivePart.info.bits.tangentVertexData)
		{
			options->AddMacroDefinition("IN_TANGENT", "1");
			options->AddMacroDefinition("IN_TANGENT_LOCATION", std::to_string(bindings++));
		}
		if (_id.model.primitivePart.info.bits.countColor)
		{
			options->AddMacroDefinition("IN_COLOR", "1");
			options->AddMacroDefinition("IN_COLOR_LOCATION", std::to_string(bindings++));
		}
		if (_id.model.primitivePart.info.bits.jointVertexData)
		{
			options->AddMacroDefinition("IN_JOINT", "1");
			options->AddMacroDefinition("IN_JOINT_LOCATION", std::to_string(bindings++));
		}
		if (_id.model.primitivePart.info.bits.weightVertexData)
		{
			options->AddMacroDefinition("IN_WEIGHT", "1");
			options->AddMacroDefinition("IN_WEIGHT_LOCATION", std::to_string(bindings++));
		}
		if (_id.model.primitivePart.info.bits.instanceMatrixData)
		{
			options->AddMacroDefinition("INSTANCE_ENABLED", "1");
			options->AddMacroDefinition("IN_MATRIX_0", std::to_string(bindings++));
			options->AddMacroDefinition("IN_MATRIX_1", std::to_string(bindings++));
			options->AddMacroDefinition("IN_MATRIX_2", std::to_string(bindings++));
		}

		bindings = 0;
		if (stage == ShaderStage::FRAGMENT)
		{
			if (_id.model.materialPart.info.bits.baseColorInfo)
			{
				options->AddMacroDefinition("BASE_COLOR", "1");
			}
			if (_id.model.materialPart.info.bits.metallicRoughnessInfo)
			{
				options->AddMacroDefinition("METALLIC_ROUGHNESS", "1");
			}
			if (_id.model.materialPart.info.bits.baseColorMap)
			{
				options->AddMacroDefinition("TEXTURE_BASE_COLOR", "1");
				options->AddMacroDefinition("TEXUTRE_BASE_COLOR_LOCATION", std::to_string(bindings++));
			}
			if (_id.model.materialPart.info.bits.normalMap)
			{
				options->AddMacroDefinition("TEXTURE_NORMAL", "1");
				options->AddMacroDefinition("TEXTURE_NORMAL_LOCATION", std::to_string(bindings++));
			}
			if (_id.model.materialPart.info.bits.metallicRoughnessMap)
			{
				options->AddMacroDefinition("TEXTURE_METALLIC_ROUGHNESS", "1");
				options->AddMacroDefinition("TEXTURE_METALLIC_ROUGHNESS_LOCATION", std::to_string(bindings++));
			}
			if (_id.model.primitivePart.info.bits.instanceMatrixData)
			{
				options->AddMacroDefinition("INSTANCE_ENABLED", "1");
				//options->AddMacroDefinition("IN_MATRIX_0", std::to_string(bindings++));
				//options->AddMacroDefinition("IN_MATRIX_1", std::to_string(bindings++));
				//options->AddMacroDefinition("IN_MATRIX_2", std::to_string(bindings++));
			}
		}
		break;
	case PipelineType::SKYBOX:
		break;
	case PipelineType::DEPTH:
		bindings = 1;
		if (stage == ShaderStage::VERTEX)
		{
			if (_id.model.primitivePart.info.bits.instanceMatrixData)
			{
				options->AddMacroDefinition("INSTANCE_ENABLED", "1");
				options->AddMacroDefinition("IN_MATRIX_0", std::to_string(bindings++));
				options->AddMacroDefinition("IN_MATRIX_1", std::to_string(bindings++));
				options->AddMacroDefinition("IN_MATRIX_2", std::to_string(bindings++));
			}
		}
		break;
	}
}

