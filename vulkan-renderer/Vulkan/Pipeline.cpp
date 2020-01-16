#include "Pipeline.h"
#include "ShaderModule.h"
#include "VulkanRenderer.h"
#include "RenderPass.h"

Pipeline::Pipeline(VulkanRenderer *renderer, PipelineId id)
{
  _id = id;
  _renderer = renderer;
}

Pipeline::~Pipeline()
{
}

void Pipeline::_addInputBinding(uint32_t stride, vk::VertexInputRate inputRate)
{
  const vk::VertexInputBindingDescription inputBinding({_inputBindings.size(),
                                                        stride,
                                                        inputRate});
  _inputBindings.push_back(std::move(inputBinding));
}

void Pipeline::_addAttributes(uint32_t location, uint32_t binding, vk::Format format, uint32_t offset)
{
  const vk::VertexInputAttributeDescription inputAttribute({
      location,
      binding,
      format,
      offset,
  });
  _inputAttributes.push_back(inputAttribute);
}

vk::DescriptorSetLayout Pipeline::_createDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
  vk::DescriptorSetLayoutCreateInfo layoutInfo({{},
                                                static_cast<uint32_t>(bindings.size()),
                                                bindings.data()});
  return _renderer->GetVulkanDevice().createDescriptorSetLayout(layoutInfo);
}

void Pipeline::InitModel()
{
  // set shader state
  {
    vk::Device device = _renderer->GetVulkanDevice();
    ShaderModule vertexShader(&device, _id);
    vertexShader.LoadFromFile("Shaders/basic_vert.spv");
    vertexShader.Build();

    ShaderModule fragmentShader(&device, _id);
    fragmentShader.LoadFromFile("Sahders/basic_frag.spv");
    fragmentShader.Build();

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
    shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());
  }

  // set vertex input state
  {
    _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
    _addAttributes(vk::Format::eR32G32B32Sfloat, 0);

    // We always have normal
    _addInputBinding(sizeof(Math::Vec3f), vk::VertexInputRate::eVertex);
    _addAttributes(vk::Format::eR32G32B32Sfloat, 0);

    // Set vertex data attributes for dynamic attributes
    if (_id.model.primitivePart.info.bits.tangentVertexData)
    {
      _addInputBinding(sizeof(Math::Vec3f), vk::VertexInputRate::eVertex);
      _addAttributes(vk::Format::eR32G32B32Sfloat, 0);
    }

    for (uint8_t i = 0; i < primitivePart.countTexCoord; ++i)
    {
      _addInputBinding(sizeof(glm::vec2), vk::VertexInputRate::eVertex);
      // _addAttributes(vk::Format::eR32G32B32Sfloat, 0);
    }

    for (uint8_t i = 0; i < primitivePart.countColor; ++i)
    {
      _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
      _addAttributes(vk::Format::eR32G32B32Sfloat, 0);
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo({{},
                                                            _inputBindings.size(),
                                                            _inputBindings.data(),
                                                            _inputAttributes.size(),
                                                            _inputAttributes.data()});
  }

  // Set input assembly state
  vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
  vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
  switch (_id.model.primitivePart.info.bits.primitiveMode)
  {
  case Render::Mesh::PrimitiveSet::Mode::Points:
    assemblyInfo = vk::PrimitiveTopology::ePointList;
    polygonMode = vk::PolygonMode::ePoint;
    break;
  case Render::Mesh::PrimitiveSet::Mode::Lines:
    assemblyInfo = vk::PrimitiveTopology::eLineList;
    polygonMode = vk::PolygonMode::eLine;
    break;
  case Render::Mesh::PrimitiveSet::Mode::LineStrip:
    assemblyInfo = vk::PrimitiveTopology::eLineStrip;
    polygonMode = vk::PolygonMode::eLine;
    break;
  case Render::Mesh::PrimitiveSet::Mode::Triangles:
    assemblyInfo = vk::PrimitiveTopology::eTriangleList;
    break;
  case Render::Mesh::PrimitiveSet::Mode::TriangleStrip:
    assemblyInfo = vk::PrimitiveTopology::eTriangleStip;
    break;
  case Render::Mesh::PrimitiveSet::Mode::TriangleFan:
    assemblyInfo = vk::PrimitiveTopology::eTriangleFan;
    break;
  }

  uint32_t width, height;
  _renderer.GetExtendSize(width, height);
  // Set viewport state
  const vk::Viewport viewport{
      /* viewport.x */ 0.0f,
      /* viewport.y */ 0.0f,
      /* viewport.width */ width,
      /* viewport.height */ height,
      /* viewport.minDepth */ 0.0f,
      /* viewport.maxDepth */ 1.0f,
  };

  const vk::Rect2D scissor{
      /* scissor.offset */ {0, 0},
      /* scissor.extent */ {width, height}};

  vk::PipelineViewportStateCreateInfo viewportState({{},
                                                     1,
                                                     &viewport,
                                                     1,
                                                     &sissor});

  // Rasterizer
  vk::PipelineRasterizationStateCreateInfo rasterizerState({{},
                                                            std::static_cast<vk::Bool32>(false),
                                                            std::static_cast<Bool32>(false),
                                                            polygonMode,
                                                            vk::CullModeFlagBits::eBack,
                                                            vk::FrontFace::eCounterClockwise,
                                                            std::static_cast<Bool32>(false),
                                                            0.0f,
                                                            0.0f,
                                                            0.0f,
                                                            1.0f});

  // Multisampling
  vk::PipelineMultisampleStateCreateInfo multisampling({{},
                                                        vk::SampleCountFlagBits::e1,
                                                        std::static_cast<vk::Bool32>(false),
                                                        float minSampleShading_ = {},
                                                        {},
                                                        std::static_cast<vk::Bool32>(false),
                                                        std::static_cast<vk::Bool32>(false)});

  // Color blending
  vk::PipelineColorBlendAttachmentState colorBlendAttachment({std::static_cast<vk::Bool32>(false),
                                                              vk::BlendFactor::eOne,
                                                              vk::BlendFactor::eZero,
                                                              vk::BlendOp::eAdd,
                                                              vk::BlendFactor::eOne,
                                                              vk::BlendFactor::eZero,
                                                              vk::BlendOp::eAdd,
                                                              vk::ColorComponentFlagBits::eR || vk::ColorComponentFlagBits::eG || vk::ColorComponentFlagBits::eB || vk::ColorComponentFlagBits::eA});

  std::array<float, 4> blendConsts = {0, 0, 0, 0};
  vk::PipelineColorBlendStateCreateInfo colorBlending({{},
                                                       std::static_cast<vk::Bool32>(false),
                                                       vk::LogicOp::eCopy,
                                                       1,
                                                       &colorBlendAttachment,
                                                       blendConsts});

  // Depth and stencil testing
  vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo({vk::PipelineDepthStencilStateCreateFlags flags_ = {},
                                                                       std::static_cast<vk::Bool32>(true),
                                                                       std::static_cast<vk::Bool32>(true),
                                                                       vk::CompareOp::eLess,
                                                                       std::static_cast<vk::Bool32>(false),
                                                                       std::static_cast<vk::Bool32>(false),
                                                                       {},
                                                                       {},
                                                                       0.0f,
                                                                       1.0f});

  // descriptor set layout
  {
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    // camera uniform buffer
    vk::DescriptorSetLayoutBinding cameraBinding({0,
                                                  vk::DescriptorType::eUniformBuffer,
                                                  1,
                                                  vk::ShaderStageFlagBits::eVertex,
                                                  {}});

    descriptorSetLayouts.push_back(_createDescriptorSetLayout({cameraBinding}));

  //   {
  //     if (_id.model.materialPart.info.bits.baseColorInfo)
  //     {
  //       vk::DescriptorSetLayoutBinding materialUniformBinding({0,
  //                                                              vk::DescriptorType::eUniformBuffer,
  //                                                              1,
  //                                                              vk::ShaderStageFlagBits::eFragment,
  //                                                              {}});
  //       descriptorSetLayouts.push_back(_createDescriptorSetLayout({materialUniformBinding}));
  //     }

  //     if (_id.model.materialPart.info.bits.metallicRoughnessInfo)
  //     {
  //     }
  //     if (_id.model.materialPart.info.bits.normalInfo)
  //     {
  //     }
  //     if (_id.model.materialPart.info.bits.occlusionInfo)
  //     {
  //     }
  //     if (_id.model.materialPart.info.bits.emissiveInfo)
  //     {
  //     }
  //   }
  //   _frameDescriptorSetLayout = descriptorSetLayouts[0];
  //   _materialDescriptorSetLayout = descriptorSetLayouts[1];
   }

  // pipeline layout
  vk::PipelineLayoutCreateInfo pipelineLayoutInfo({{},
                                                   descriptorSetLayouts.size(),
                                                   descriptorSetLayouts.data(),
                                                   0,
                                                   {}});
  _pipelineLayout = _device.createPipelineLayout(pipelineInfo);


  vk::GraphicsPipelineCreateInfo pipelineInfo({{},
                                               shaderStages.size(),
                                               shaderStages.data(),
                                               &vertexInputInfo,
                                               &assemblyInfo,
                                               {},
                                               &viewportState,
                                               &rasterizerState,
                                               &multiSampling,
                                               &depthStencilStateCreateInfo,
                                               colorBlending,
                                               {},
                                               _pipelineLayout,
                                               _renderer->GetRenderPass(),
                                               uint32_t subpass_ = {},
                                               {},
                                               {}});

  _graphicsPipeline = _device.createGraphicsPipeline(pipelineInfo);
}

vk::RenderPass Pipeline::GetRenderPass()
{
  return _renderPass;
}
vk::Pipeline Pipeline::GetPipeline()
{
  return _graphicsPipeline;
}

vk::PipelineLayout Pipeline::GetPipelineLayout()
{
  return _pipelineLayout;
}
