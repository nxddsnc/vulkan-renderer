#include "Pipeline.h"
#include "ShaderModule.h"

Pipeline::Pipeline(PipelineId id)
{
  this._id = id;
}

Pipeline::~Pipeline()
{
}

Pipleline::InitModel()
{
  ShaderModule vertexShader;
  vertexShader.LoadFromFile("Shaders/basic_vert.spv");
  vertexShader.build();

  ShaderModule fragmentShader;
  fragmentShader.LoadFromFile("Sahders/basic_frag.spv");
  fragmentShader.build();

	vk::GraphicsPipelineCreateInfo createInfo({
    {},
    uint32_t stageCount_ = {},
    const VULKAN_HPP_NAMESPACE::PipelineShaderStageCreateInfo* pStages_ = {},
    const VULKAN_HPP_NAMESPACE::PipelineVertexInputStateCreateInfo* pVertexInputState_ = {},
    const VULKAN_HPP_NAMESPACE::PipelineInputAssemblyStateCreateInfo* pInputAssemblyState_ = {},
    const VULKAN_HPP_NAMESPACE::PipelineTessellationStateCreateInfo* pTessellationState_ = {},
    const VULKAN_HPP_NAMESPACE::PipelineViewportStateCreateInfo* pViewportState_ = {},
    const VULKAN_HPP_NAMESPACE::PipelineRasterizationStateCreateInfo* pRasterizationState_ = {},
    const VULKAN_HPP_NAMESPACE::PipelineMultisampleStateCreateInfo* pMultisampleState_ = {},
    const VULKAN_HPP_NAMESPACE::PipelineDepthStencilStateCreateInfo* pDepthStencilState_ = {},
    const VULKAN_HPP_NAMESPACE::PipelineColorBlendStateCreateInfo* pColorBlendState_ = {},
    const VULKAN_HPP_NAMESPACE::PipelineDynamicStateCreateInfo* pDynamicState_ = {},
    VULKAN_HPP_NAMESPACE::PipelineLayout layout_ = {},
    VULKAN_HPP_NAMESPACE::RenderPass renderPass_ = {},
    uint32_t subpass_ = {},
    VULKAN_HPP_NAMESPACE::Pipeline basePipelineHandle_ = {},
    int32_t basePipelineIndex_ = {} 
  });
  pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline);
}