#include "Platform.h"
#pragma once
enum PipelineType
{
	MODEL_FORWARD,
	MODEL_DEFERRED,
	DEPTH,
	DEFERRED_SHADING,
	SKYBOX,
	PREFILTERED_CUBE_MAP,
	IRRADIANCE_MAP,
	GENERATE_BRDF_LUT,
	BRIGHTNESS,
	GAUSSIAN_BLUR_X,
	GAUSSIAN_BLUR_Y,
	BLIT
};

enum class PrimitiveMode : uint8_t {
    Points = 0,         ///< Each vertex defines a separate point
    Lines = 1,          ///< The first two vertices define the first segment, with subsequent pairs of vertices each defining one more segment
    LineStrip = 3,      ///< The first vertex specifies the first segments start point while the second vertex specifies the first segments endpoint and the second segments start point
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
          bool positionVertexData : 1;
          bool normalVertexData : 1;
		  bool tangentVertexData : 1;
		  bool jointVertexData : 1;
		  bool weightVertexData : 1;
		  bool instanceMatrixData : 1;
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
          bool baseColorMap:                   1;
          bool normalMap:                      1;
          bool metallicRoughnessMap:           1;
        } bits;

        uint32_t value;
      } info;
    } materialPart;
  } model;

  bool operator==(PipelineId const & rhs) const
  {
      return type == rhs.type && model.materialPart.info.value == rhs.model.materialPart.info.value &&
          model.primitivePart.info.value == rhs.model.primitivePart.info.value;
  }

  bool operator!=(PipelineId const & rhs) const
  {
      return type != rhs.type || model.materialPart.info.value != rhs.model.materialPart.info.value ||
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
class MyCamera;
class RenderPass;
class Pipeline
{
public:
    Pipeline(VulkanRenderer *renderer, PipelineId id);
    Pipeline(PipelineId id);
    ~Pipeline();

    void Destroy();
    vk::Pipeline GetPipeline();
    vk::PipelineLayout GetPipelineLayout();
    void InitModelForward(std::shared_ptr<RenderPass> renderPass);
	void InitDepth(std::shared_ptr<RenderPass> renderPass);
	void InitModelGBuffer(std::shared_ptr<RenderPass> renderPass);
    void InitSkybox(std::shared_ptr<RenderPass> renderPass);
    void InitPrefilteredCubeMap(vk::Device device, vk::RenderPass renderPass);
    void InitIrradianceMap(vk::Device device, vk::RenderPass renderPass);
    void InitGenerateBrdfLut(vk::Device device, vk::RenderPass renderPass);
	void InitBrightPass(vk::Device device, vk::RenderPass renderPass);
	void InitGaussianBlur(vk::Device device, vk::RenderPass renderPass);
	void InitBlit(vk::Device device, vk::RenderPass renderPass);
	void InitDeferred(std::shared_ptr<RenderPass> renderPass);
public:
	bool m_bReady;
	PipelineId m_id;
private:
    VulkanRenderer    * _renderer;
    vk::Device          m_device;
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