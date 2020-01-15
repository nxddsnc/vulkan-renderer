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

    vk::RenderPass GetRenderPass();
    vk::Pipeline GetPipeline();
    vk::PipelineLayout GetPipelineLayout();
    vk::DescriptorSetLayout GetFrameDescriptorSetLayout();
    vk::DescriptorSetLayout GetMaterialDescritporSetLayout();
    vk::DescriptorSetLayout GetMaterialImageDescriptorSetLayout();
    _addAttributes(uint32_t location, vk::Format format, uint32_t offset);
private:
    PipelineId _id;
    VulkanRenderer    * renderer;
    vk::RenderPass      _renderPass;
    vk::Pipeline        _graphicsPipeline;
    vk::PipelineLayout  _pipelineLayout;
    vk::DescriptorSetLayout _materialDescriptorSetLayout;
    vk::DescriptorSetLayout _materialImageDescriptorSetLayout;

    // Vertex input state
    std::vector<vk::VertexInputBindingDescription> _inputBindings;
    std::vector<vk::VertexInputAttributeDescription> _inputAttributes;
};