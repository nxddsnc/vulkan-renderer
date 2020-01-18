#include "Platform.h"
#pragma once
enum PipelineType
{
  MODEL
};

enum class PrimitiveMode : uint8_t {
    Points = 0,         ///< Each vertex defines a separate point
    Lines = 1,          ///< The first two vertices define the first segment, with subsequent pairs of vertices each defining one more segment
    LineStrip = 3,      ///< The first vertex specifies the first segment��s start point while the second vertex specifies the first segment��s endpoint and the second segment��s start point
    Triangles = 4,      ///<
    TriangleStrip = 5,  ///<
    TriangleFan = 6     ///<
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
          uint8_t positionVertexData : 1;
          uint8_t normalVertexData : 1;
          uint8_t tangentVertexData : 1;
          uint8_t countTexCoord : 2;
          uint8_t countColor : 2;
          PrimitiveMode primitiveMode : 3;
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

  bool operator==(PipelineId const & rhs) const
  {
      return model.materialPart.info.value == rhs.model.materialPart.info.value &&
          model.primitivePart.info.value == rhs.model.primitivePart.info.value;
  }

  bool operator!=(PipelineId const & rhs) const
  {
      return model.materialPart.info.value != rhs.model.materialPart.info.value ||
          model.primitivePart.info.value != rhs.model.primitivePart.info.value;
  }
};

template<> struct std::hash<PipelineId> {
    size_t operator()(const PipelineId& id) const {
        // TODO: use more appropriate hash value
        return std::hash<uint32_t>()(id.model.primitivePart.info.value * 31 + id.model.materialPart.info.value);
    }
};

class VulkanRenderer;
class VulkanCamera;
class Pipeline
{
public:
    Pipeline(VulkanRenderer *renderer, PipelineId id);
    ~Pipeline();

    void Destroy();
    vk::Pipeline GetPipeline();
    vk::PipelineLayout GetPipelineLayout();
    void InitModel();
private:
    PipelineId _id;
    VulkanRenderer    * _renderer;
    vk::RenderPass      _renderPass;
    vk::Pipeline        _graphicsPipeline;
    vk::PipelineLayout  _pipelineLayout;
    vk::DescriptorSetLayout _materialDescriptorSetLayout;
    vk::DescriptorSetLayout _materialImageDescriptorSetLayout;
    vk::DescriptorSet       _cameraDescriptorSet;

    // Vertex input state
    std::vector<vk::VertexInputBindingDescription> _inputBindings;
    std::vector<vk::VertexInputAttributeDescription> _inputAttributes;
private:
    void _addAttributes(uint32_t location, uint32_t binding, vk::Format format, uint32_t offset);
    vk::DescriptorSetLayout _createDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> bindings);
    void _addInputBinding(uint32_t stride, vk::VertexInputRate inputRate);
};