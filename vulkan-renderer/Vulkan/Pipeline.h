#include "Platform.h"
#pragma once
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

  bool operator==(PipelineId const & rhs) const VULKAN_HPP_NOEXCEPT
  {
      return model.materialPart.info.value == rhs.model.materialPart.info.value &&
          model.primitivePart.info.value == rhs.model.primitivePart.info.value;
  }

  bool operator!=(PipelineId const & rhs) const VULKAN_HPP_NOEXCEPT
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

    vk::RenderPass GetRenderPass();
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