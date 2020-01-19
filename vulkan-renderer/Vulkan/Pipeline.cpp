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

void Pipeline::Destroy()
{
    _renderer->GetVulkanDevice().destroyPipeline(_graphicsPipeline);
    _renderer->GetVulkanDevice().destroyPipelineLayout(_pipelineLayout);
}

void Pipeline::_addInputBinding(uint32_t stride, vk::VertexInputRate inputRate)
{
    const vk::VertexInputBindingDescription inputBinding({ static_cast<uint32_t>(_inputBindings.size()),
                                                          stride,
                                                          inputRate });
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
    vk::DescriptorSetLayoutCreateInfo layoutInfo({ {},
                                                  static_cast<uint32_t>(bindings.size()),
                                                  bindings.data() });
    return _renderer->GetVulkanDevice().createDescriptorSetLayout(layoutInfo);
}

void Pipeline::InitModel()
{
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    // set shader state
    vk::Device device = _renderer->GetVulkanDevice();
    ShaderModule vertexShader(&device, _id);
    vertexShader.BuildFromFile("Shaders/basic.vert", ShaderStage::VERTEX, "main");

    ShaderModule fragmentShader(&device, _id);
    fragmentShader.BuildFromFile("Shaders/basic.frag", ShaderStage::FRAGMENT, "main");

    shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
    shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

    // set vertex input state
    _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
    _addAttributes(0, 0, vk::Format::eR32G32B32Sfloat, 0);

    // We always have normal
    _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
    _addAttributes(1, 1, vk::Format::eR32G32B32Sfloat, 0);

    // Set vertex data attributes for dynamic attributes
    if (_id.model.primitivePart.info.bits.countTexCoord > 0)
    {
        _addInputBinding(sizeof(glm::vec2), vk::VertexInputRate::eVertex);
        _addAttributes(2, 2, vk::Format::eR32G32Sfloat, 0);
    }

    if (_id.model.primitivePart.info.bits.tangentVertexData)
    {
        _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
        _addAttributes(3, 3, vk::Format::eR32G32B32Sfloat, 0);
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
                                                            static_cast<uint32_t>(_inputBindings.size()),
                                                            _inputBindings.data(),
                                                            static_cast<uint32_t>(_inputAttributes.size()),
                                                            _inputAttributes.data() });

    // Set input assembly state
    vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
    vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
    switch (_id.model.primitivePart.info.bits.primitiveMode)
    {
    case PrimitiveMode::Points:
        assemblyInfo.topology = vk::PrimitiveTopology::ePointList;
        polygonMode = vk::PolygonMode::ePoint;
        break;
    case PrimitiveMode::Lines:
        assemblyInfo.topology = vk::PrimitiveTopology::eLineList;
        polygonMode = vk::PolygonMode::eLine;
        break;
    case PrimitiveMode::LineStrip:
        assemblyInfo.topology = vk::PrimitiveTopology::eLineStrip;
        polygonMode = vk::PolygonMode::eLine;
        break;
    case PrimitiveMode::Triangles:
        assemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
        break;
    case PrimitiveMode::TriangleStrip:
        assemblyInfo.topology = vk::PrimitiveTopology::eTriangleStrip;
        break;
    case PrimitiveMode::TriangleFan:
        assemblyInfo.topology = vk::PrimitiveTopology::eTriangleFan;
        break;
    }

    uint32_t width, height;
    _renderer->GetExtendSize(width, height);
    // Set viewport state
    const vk::Viewport viewport{
        /* viewport.x */ 0.0f,
        /* viewport.y */ 0.0f,
        /* viewport.width */  (float)width,
        /* viewport.height */ (float)height,
        /* viewport.minDepth */ 0.0f,
        /* viewport.maxDepth */ 1.0f,
    };

    const vk::Rect2D scissor{
        /* scissor.offset */ {0, 0},
        /* scissor.extent */ {width, height} };

    vk::PipelineViewportStateCreateInfo viewportState({ {},
                                                       1,
                                                       &viewport,
                                                       1,
                                                       &scissor });

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizerState({ {},
                                                              static_cast<vk::Bool32>(false),
                                                              static_cast<vk::Bool32>(false),
                                                              polygonMode,
                                                              vk::CullModeFlagBits::eBack,
                                                              vk::FrontFace::eCounterClockwise,
                                                              static_cast<vk::Bool32>(false),
                                                              0.0f,
                                                              0.0f,
                                                              0.0f,
                                                              1.0f });

    // Multisampling
    vk::PipelineMultisampleStateCreateInfo multisampling({ {},
                                                          vk::SampleCountFlagBits::e1,
                                                          static_cast<vk::Bool32>(false),
                                                          {},
                                                          {},
                                                          static_cast<vk::Bool32>(false),
                                                          static_cast<vk::Bool32>(false) });

    // Color blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachment({ static_cast<vk::Bool32>(false),
                                                                vk::BlendFactor::eOne,
                                                                vk::BlendFactor::eZero,
                                                                vk::BlendOp::eAdd,
                                                                vk::BlendFactor::eOne,
                                                                vk::BlendFactor::eZero,
                                                                vk::BlendOp::eAdd,
                                                                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA });

    std::array<float, 4> blendConsts = { 0, 0, 0, 0 };
    vk::PipelineColorBlendStateCreateInfo colorBlending({ {},
                                                         static_cast<vk::Bool32>(false),
                                                         vk::LogicOp::eCopy,
                                                         1,
                                                         &colorBlendAttachment,
                                                         blendConsts });

    // Depth and stencil testing
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo({ {},
                                                                         static_cast<vk::Bool32>(true),
                                                                         static_cast<vk::Bool32>(true),
                                                                         vk::CompareOp::eLess,
                                                                         static_cast<vk::Bool32>(false),
                                                                         static_cast<vk::Bool32>(false),
                                                                         {},
                                                                         {},
                                                                         0.0f,
                                                                         1.0f });

    // descriptor set layout
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    // camera uniform buffer
    vk::DescriptorSetLayoutBinding cameraBinding({ 0,
                                                  vk::DescriptorType::eUniformBuffer,
                                                  1,
                                                  vk::ShaderStageFlagBits::eVertex,
                                                  {} });

    descriptorSetLayouts.push_back(_createDescriptorSetLayout({ cameraBinding }));

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
     //}

    // pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({ {},
                                                     static_cast<uint32_t>(descriptorSetLayouts.size()),
                                                     descriptorSetLayouts.data(),
                                                     0,
                                                     {} });
    _pipelineLayout = _renderer->GetVulkanDevice().createPipelineLayout(pipelineLayoutInfo);

    vk::GraphicsPipelineCreateInfo pipelineInfo({ {},
                                                 static_cast<uint32_t>(shaderStages.size()),
                                                 shaderStages.data(),
                                                 &vertexInputInfo,
                                                 &assemblyInfo,
                                                 {},
                                                 &viewportState,
                                                 &rasterizerState,
                                                 &multisampling,
                                                 &depthStencilStateCreateInfo,
                                                 &colorBlending,
                                                 {},
                                                 _pipelineLayout,
                                                 _renderer->GetRenderPass(),
                                                 {},
                                                 {},
                                                 static_cast<int32_t>(-1) });

    _graphicsPipeline = _renderer->GetVulkanDevice().createGraphicsPipeline(nullptr, pipelineInfo);

    for (auto descriptorSetLayout : descriptorSetLayouts)
    {
        _renderer->GetVulkanDevice().destroyDescriptorSetLayout(descriptorSetLayout);
    }
}

vk::Pipeline Pipeline::GetPipeline()
{
    return _graphicsPipeline;
}

vk::PipelineLayout Pipeline::GetPipelineLayout()
{
    return _pipelineLayout;
}
