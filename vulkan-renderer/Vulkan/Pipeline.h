
enum PipelineType
{
  MODEL
};

struct PipelineId
{
  PipelineType type;
  struct Model
  {
    struct PrimitivePart
    {
      union {
        struct
        {
          unsigned int positionVertexData : 1;
          unsigned int normalVertexData : 1;
          unsigned int tangentVertexData : 1;
          unsigned int countTexCoord : 2;
          unsigned int countColor : 2;
          unsigned int primitiveMode : 3;
        } bits;
        uint32_t value;
      } info;
    } primitivePart;
    struct MaterialPart
    {
      union {
        struct
        {
          unsigned int baseColorInfo:          2;
          unsigned int metallicRoughnessInfo:  2;
          unsigned int normalInfo:             2;
          unsigned int occlusionInfo:          2;
          unsigned int emissiveInfo:           2;
        } bits;

        uint32_t value;
      } info;
    } materialPart;
  } model;
};

class Renderer;
class Pipeline
{
public:
    Pipeline(Renderer *renderer, PipelineId id);
    ~Pipeline();

    _addAttributes(uint32_t location, vk::Format format, uint32_t offset);
private:
    PipelineId _id;

    VulkanRenderer   *renderer;
  
    RenderPass  _renderPass;

    // Vertex input state
    std::vector<vk::VertexInputBindingDescription> _inputBindings;
    std::vector<vk::VertexInputAttributeDescription> _inputAttributes;

    // Input assembly state
    VkPrimitiveTopology _topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    bool _primitiveRestartEnable{VK_FALSE};

    // Viewport state
    VkPipelineViewportStateCreateInfo _viewportState{
        /* _viewportState.sType */ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        /* _viewportState.pNext */ nullptr,
        /* _viewportState.flags */ 0,
        /* _viewportState.viewportCount */ 0, // Will be set to _viewports.size()
        /* _viewportState.pViewports */ nullptr, // Will be set to _viewports.data()
        /* _viewportState.scissorCount */ 0, // Will be set to _scissors.size()
        /* _viewportState.pScissors */ nullptr // Will be set to _scissors.data()
    };

    std::vector<VkViewport> _viewports;
    std::vector<VkRect2D> _scissors;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo _rasterizationState{
        /* _rasterizationState.sType */ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        /* _rasterizationState.pNext */ nullptr,
        /* _rasterizationState.flags */ 0,
        /* _rasterizationState.depthClampEnable */ VK_FALSE,
        /* _rasterizationState._rasterizationStateDiscardEnable */ VK_FALSE,
        /* _rasterizationState.polygonMode */ VK_POLYGON_MODE_FILL,
        /* _rasterizationState.cullMode */ VK_CULL_MODE_BACK_BIT,
        /* _rasterizationState.frontFace */ VK_FRONT_FACE_COUNTER_CLOCKWISE,
        /* _rasterizationState.depthBiasEnable */ VK_FALSE,
        /* _rasterizationState.depthBiasConstantFactor */ 0.0f,
        /* _rasterizationState.depthBiasClamp */ 0.0f,
        /* _rasterizationState.depthBiasSlopeFactor */ 0.0f,
        /* _rasterizationState.lineWidth */ 1.0f
    };

    // Multisample state
    VkPipelineMultisampleStateCreateInfo _multisampleState{
        /* _multisampleState.sType */ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        /* _multisampleState.pNext */ nullptr,
        /* _multisampleState.flags */ 0,
        /* _multisampleState.rasterizationSamples */ VK_SAMPLE_COUNT_1_BIT,
        /* _multisampleState.sampleShadingEnable */ VK_FALSE,
        /* _multisampleState.minSampleShading */ 0.0f,
        /* _multisampleState.pSampleMask */ nullptr,
        /* _multisampleState.alphaToCoverageEnable */ VK_FALSE,
        /* _multisampleState.alphaToOneEnable */ VK_FALSE
    };

    // Depth and Stencil state
    VkPipelineDepthStencilStateCreateInfo _depthStencilState{
        /* _depthStencilState.sType */ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        /* _depthStencilState.pNext */ nullptr,
        /* _depthStencilState.flags */ 0,
        /* _depthStencilState.depthTestEnable */ VK_FALSE,
        /* _depthStencilState.depthWriteEnable */ VK_FALSE,
        /* _depthStencilState.depthCompareOp */ VK_COMPARE_OP_NEVER,
        /* _depthStencilState.depthBoundsTestEnable */ VK_FALSE,
        /* _depthStencilState.stencilTestEnable */ VK_FALSE,
        /* _depthStencilState.front */ {},
        /* _depthStencilState.back */ {},
        /* _depthStencilState.minDepthBounds */ 0.0f,
        /* _depthStencilState.maxDepthBounds */ 0.0f
    };

    // Color Blend state
    VkPipelineColorBlendStateCreateInfo _colorBlendState{
        /* _colorBlendState.sType */ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        /* _colorBlendState.pNext */ nullptr,
        /* _colorBlendState.flags */ 0,
        /* _colorBlendState.logicOpEnable */ VK_FALSE,
        /* _colorBlendState.logicOp */ VK_LOGIC_OP_CLEAR,
        /* _colorBlendState.attachmentCount */ 0, // Will be set to _colorBlendAttachments.size()
        /* _colorBlendState.pAttachments */ nullptr, // Will be set to _colorBlendAttachments.data()
        /* _colorBlendState.blendConstants */ {0.0f, 0.0f, 0.0f, 0.0f}
    };

    std::vector<VkPipelineColorBlendAttachmentState> _colorBlendAttachments;
};