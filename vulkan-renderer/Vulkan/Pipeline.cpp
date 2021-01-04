#include "Pipeline.h"
#include "ShaderModule.h"
#include "VulkanRenderer.h"
#include "RenderPass.h"

Pipeline::Pipeline(VulkanRenderer *renderer, PipelineId id)
{
    m_id = id;
    _renderer = renderer;
    m_device = renderer->GetVulkanDevice();
	m_bReady = false;
}

Pipeline::Pipeline(PipelineId id)
{
    m_id = id;
}

Pipeline::~Pipeline()
{
    Destroy();
}

void Pipeline::Destroy()
{
    if (_graphicsPipeline)
    {
        m_device.destroyPipeline(_graphicsPipeline);
        _graphicsPipeline = nullptr;
    }
    
    if (_pipelineLayout)
    {
        m_device.destroyPipelineLayout(_pipelineLayout);
        _pipelineLayout = nullptr;
    }
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
    return m_device.createDescriptorSetLayout(layoutInfo);
}

void Pipeline::InitModelForward(std::shared_ptr<RenderPass> renderPass)
{
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    vk::Device device = m_device;
    
    // set shader state
    char vertexShaderName[32] = "pbr";
    char fragmentShaderName[32] = "pbr";
    switch (m_id.model.primitivePart.info.bits.primitiveMode)
    {
    case PrimitiveMode::Lines:
        strcpy(fragmentShaderName, "plain");
        strcpy(vertexShaderName, "basic");
        break;
    }
    char vertexShaderPath[128];
    char fragmenentShaderPath[128];
    sprintf(vertexShaderPath, "Shaders/%s.vert", vertexShaderName);
    sprintf(fragmenentShaderPath, "Shaders/%s.frag", fragmentShaderName);
    ShaderModule vertexShader(&device, m_id);
    vertexShader.BuildFromFile(vertexShaderPath, ShaderStage::VERTEX, "main");

    ShaderModule fragmentShader(&device, m_id);
    fragmentShader.BuildFromFile(fragmenentShaderPath, ShaderStage::FRAGMENT, "main");

    shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
    shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

    // set vertex input state
    _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
    _addAttributes(0, 0, vk::Format::eR32G32B32Sfloat, 0);

    uint32_t attributeIndex = 1;
    if (m_id.model.primitivePart.info.bits.normalVertexData)
    {
        _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
        _addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32Sfloat, 0);
        attributeIndex++;
    }

    // Set vertex data attributes for dynamic attributes
    if (m_id.model.primitivePart.info.bits.countTexCoord)
    {
        _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
        _addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32Sfloat, 0);
        attributeIndex++;
    }

    if (m_id.model.primitivePart.info.bits.tangentVertexData)
    {
        _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
        _addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32Sfloat, 0);
        attributeIndex++;
    }

    if (m_id.model.primitivePart.info.bits.countColor)
    {
        _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
        _addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32Sfloat, 0);
        attributeIndex++;
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
                                                            static_cast<uint32_t>(_inputBindings.size()),
                                                            _inputBindings.data(),
                                                            static_cast<uint32_t>(_inputAttributes.size()),
                                                            _inputAttributes.data() });

    // Set input assembly state
    vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
    vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
    switch (m_id.model.primitivePart.info.bits.primitiveMode)
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

    // TODO: Extract the descriptorSetlayout part as a single class.
    // descriptor set layout
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    // camera uniform buffer
    vk::DescriptorSetLayoutBinding cameraBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, {});

    descriptorSetLayouts.push_back(_createDescriptorSetLayout({ cameraBinding }));

    // light uniform buffer.
    vk::DescriptorSetLayoutBinding lightBidning(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment, {});
    descriptorSetLayouts.push_back(_createDescriptorSetLayout({ lightBidning }));

    // Prefiltered environment map and brdf look up table.
    vk::DescriptorSetLayoutBinding preFileteredEnvMapBinding(0, vk::DescriptorType::eCombinedImageSampler,
        1, vk::ShaderStageFlagBits::eFragment, {});
    vk::DescriptorSetLayoutBinding irradianceMapBinding(1, vk::DescriptorType::eCombinedImageSampler,
        1, vk::ShaderStageFlagBits::eFragment, {});
    vk::DescriptorSetLayoutBinding brdfLutBinding(2, vk::DescriptorType::eCombinedImageSampler,
        1, vk::ShaderStageFlagBits::eFragment, {});
    descriptorSetLayouts.push_back(_createDescriptorSetLayout({ preFileteredEnvMapBinding, irradianceMapBinding, brdfLutBinding }));

    // per renderable
    std::vector<vk::DescriptorSetLayoutBinding> perRenderableBindings;
    
    std::vector<vk::PushConstantRange> pushConstantRanges;

    uint32_t offset = 0;
    uint32_t size = sizeof(glm::mat4) * 2;
    pushConstantRanges.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex, offset, size));
    offset += size;
    
    size = 0;
    if (m_id.model.materialPart.info.bits.baseColorInfo)
    {
        size += sizeof(glm::vec4);
    }
    if (m_id.model.materialPart.info.bits.metallicRoughnessInfo)
    {
        size += sizeof(glm::vec2);
    }
    pushConstantRanges.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment, offset, size));

    uint32_t bindings = 0;
    if (m_id.model.materialPart.info.bits.baseColorMap)
    {
        vk::DescriptorSetLayoutBinding materialSamplerBinding(bindings++,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        perRenderableBindings.push_back(materialSamplerBinding);
    }
    if (m_id.model.materialPart.info.bits.normalMap)
    {
        vk::DescriptorSetLayoutBinding materialSamplerBinding(bindings++,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        perRenderableBindings.push_back(materialSamplerBinding);
    }
    if (m_id.model.materialPart.info.bits.metallicRoughnessMap)
    {
        vk::DescriptorSetLayoutBinding materialSamplerBinding(bindings++,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        perRenderableBindings.push_back(materialSamplerBinding);
    }
	if (perRenderableBindings.size() > 0)
	{
		descriptorSetLayouts.push_back(_createDescriptorSetLayout(perRenderableBindings));
	}

    // pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({ {},
                                                     static_cast<uint32_t>(descriptorSetLayouts.size()),
                                                     descriptorSetLayouts.data(),
                                                     static_cast<uint32_t>(pushConstantRanges.size()),
                                                     pushConstantRanges.data() });
    _pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

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
                                                 renderPass->Get(),
                                                 {},
                                                 {},
                                                 static_cast<int32_t>(-1) });

    _graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

    for (auto descriptorSetLayout : descriptorSetLayouts)
    {
        m_device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
}

void Pipeline::InitDepth(std::shared_ptr<RenderPass> renderPass)
{
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	vk::Device device = m_device;

	// set shader state
	char vertexShaderName[32] = "pbr";
	char fragmentShaderName[32] = "depth";

	char vertexShaderPath[128];
	char fragmenentShaderPath[128];
	sprintf(vertexShaderPath, "Shaders/%s.vert", vertexShaderName);
	sprintf(fragmenentShaderPath, "Shaders/%s.frag", fragmentShaderName);
	ShaderModule vertexShader(&device, m_id);
	vertexShader.BuildFromFile(vertexShaderPath, ShaderStage::VERTEX, "main");

	ShaderModule fragmentShader(&device, m_id);
	fragmentShader.BuildFromFile(fragmenentShaderPath, ShaderStage::FRAGMENT, "main");

	shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
	shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

	// set vertex input state
	_addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
	_addAttributes(0, 0, vk::Format::eR32G32B32Sfloat, 0);

	if (m_id.model.primitivePart.info.bits.instanceMatrixData)
	{
		for (int i = 0; i < 3; ++i)
		{
			_addInputBinding(sizeof(float) * 4, vk::VertexInputRate::eInstance);
			_addAttributes(i + 1, i + 1, vk::Format::eR32G32B32A32Sfloat, 0);
		}
	}

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
		static_cast<uint32_t>(_inputBindings.size()),
		_inputBindings.data(),
		static_cast<uint32_t>(_inputAttributes.size()),
		_inputAttributes.data() });

	// Set input assembly state
	vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
	vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
	switch (m_id.model.primitivePart.info.bits.primitiveMode)
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
		/* scissor.offset */{ 0, 0 },
		/* scissor.extent */{ width, height } };

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
		static_cast<vk::Bool32>(true),
		8.0f,
		0.0f,
		8.0f,
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

	// TODO: Extract the descriptorSetlayout part as a single class.
	// descriptor set layout
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	// camera uniform buffer
	vk::DescriptorSetLayoutBinding cameraBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, {});

	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ cameraBinding }));

	// per renderable
	std::vector<vk::DescriptorSetLayoutBinding> perRenderableBindings;

	std::vector<vk::PushConstantRange> pushConstantRanges;

	uint32_t offset = 0;
	uint32_t size = sizeof(glm::mat4) * 2;
	if (!m_id.model.primitivePart.info.bits.instanceMatrixData)
	{
		pushConstantRanges.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex, offset, size));
		offset += size;
	}

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({ {},
		static_cast<uint32_t>(descriptorSetLayouts.size()),
		descriptorSetLayouts.data(),
		static_cast<uint32_t>(pushConstantRanges.size()),
		pushConstantRanges.data()});
	_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

	std::array<vk::DynamicState, 2> dynamicStates;
	dynamicStates[0] = vk::DynamicState::eViewport;
	dynamicStates[1] = vk::DynamicState::eScissor;
	//dynamicStates[2] = vk::DynamicState::eDepthBias;
	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates.size(), dynamicStates.data());

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
		&dynamicStateCreateInfo,
		_pipelineLayout,
		renderPass->Get(),
		{},
		{},
		static_cast<int32_t>(-1) });

	_graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

	for (auto descriptorSetLayout : descriptorSetLayouts)
	{
		m_device.destroyDescriptorSetLayout(descriptorSetLayout);
	}
}

void Pipeline::InitModelGBuffer(std::shared_ptr<RenderPass> renderPass)
{
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	vk::Device device = m_device;

	// set shader state
	char vertexShaderName[32] = "pbr";
	char fragmentShaderName[32] = "gBuffer";

	char vertexShaderPath[128];
	char fragmenentShaderPath[128];
	sprintf(vertexShaderPath, "Shaders/%s.vert", vertexShaderName);
	sprintf(fragmenentShaderPath, "Shaders/%s.frag", fragmentShaderName);
	ShaderModule vertexShader(&device, m_id);
	vertexShader.BuildFromFile(vertexShaderPath, ShaderStage::VERTEX, "main");

	ShaderModule fragmentShader(&device, m_id);
	fragmentShader.BuildFromFile(fragmenentShaderPath, ShaderStage::FRAGMENT, "main");

	shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
	shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

	// set vertex input state
	_addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
	_addAttributes(0, 0, vk::Format::eR32G32B32Sfloat, 0);

	uint32_t attributeIndex = 1;
	if (m_id.model.primitivePart.info.bits.normalVertexData)
	{
		_addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
		_addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32Sfloat, 0);
		attributeIndex++;
	}

	// Set vertex data attributes for dynamic attributes
	if (m_id.model.primitivePart.info.bits.countTexCoord)
	{
		_addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
		_addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32Sfloat, 0);
		attributeIndex++;
	}

	if (m_id.model.primitivePart.info.bits.tangentVertexData)
	{
		_addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
		_addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32Sfloat, 0);
		attributeIndex++;
	}

	if (m_id.model.primitivePart.info.bits.countColor)
	{
		_addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
		_addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32Sfloat, 0);
		attributeIndex++;
	}

	if (m_id.model.primitivePart.info.bits.jointVertexData)
	{
		_addInputBinding(sizeof(glm::vec4), vk::VertexInputRate::eVertex);
		_addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32A32Sint, 0);
		attributeIndex++;
	}

	if (m_id.model.primitivePart.info.bits.weightVertexData)
	{
		_addInputBinding(sizeof(glm::vec4), vk::VertexInputRate::eVertex);
		_addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32A32Sfloat, 0);
		attributeIndex++;
	}

	if (m_id.model.primitivePart.info.bits.instanceMatrixData)
	{
		for (int i = 0; i < 3; ++i)
		{
			_addInputBinding(sizeof(float) * 4, vk::VertexInputRate::eInstance);
			_addAttributes(attributeIndex, attributeIndex, vk::Format::eR32G32B32A32Sfloat, 0);
			attributeIndex++;
		}
	}

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
		static_cast<uint32_t>(_inputBindings.size()),
		_inputBindings.data(),
		static_cast<uint32_t>(_inputAttributes.size()),
		_inputAttributes.data() });

	// Set input assembly state
	vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
	vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
	switch (m_id.model.primitivePart.info.bits.primitiveMode)
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
		/* scissor.offset */{ 0, 0 },
		/* scissor.extent */{ width, height } };

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

	std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachements;
	for (int i = 0; i < renderPass->m_colorAttachments.size(); ++i)
	{
		// Color blending
		vk::PipelineColorBlendAttachmentState colorBlendAttachment(static_cast<vk::Bool32>(false),
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

		colorBlendAttachements.push_back(colorBlendAttachment);
	}

	std::array<float, 4> blendConsts = { 0, 0, 0, 0 };
	vk::PipelineColorBlendStateCreateInfo colorBlending({},
		static_cast<vk::Bool32>(false),
		vk::LogicOp::eCopy,
		(uint32_t)colorBlendAttachements.size(),
		colorBlendAttachements.data(),
		blendConsts);

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

	// TODO: Extract the descriptorSetlayout part as a single class.
	// descriptor set layout
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	// camera uniform buffer
	vk::DescriptorSetLayoutBinding cameraBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, {});

	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ cameraBinding }));

	// per renderable
	std::vector<vk::DescriptorSetLayoutBinding> perRenderableBindings;

	std::vector<vk::PushConstantRange> pushConstantRanges;

	uint32_t offset = 0;
	uint32_t size = sizeof(glm::mat4) * 2;
	if (!m_id.model.primitivePart.info.bits.instanceMatrixData)
	{
		pushConstantRanges.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex, offset, size));
		offset += size;
	}

	size = 0;
	if (m_id.model.materialPart.info.bits.baseColorInfo)
	{
		size += sizeof(glm::vec4);
	}
	if (m_id.model.materialPart.info.bits.metallicRoughnessInfo)
	{
		size += sizeof(glm::vec2);
	}
	pushConstantRanges.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment, offset, size));

	uint32_t bindings = 0;
	if (m_id.model.materialPart.info.bits.baseColorMap)
	{
		vk::DescriptorSetLayoutBinding materialSamplerBinding(bindings++,
			vk::DescriptorType::eCombinedImageSampler,
			1,
			vk::ShaderStageFlagBits::eFragment,
			{});
		perRenderableBindings.push_back(materialSamplerBinding);
	}
	if (m_id.model.materialPart.info.bits.normalMap)
	{
		vk::DescriptorSetLayoutBinding materialSamplerBinding(bindings++,
			vk::DescriptorType::eCombinedImageSampler,
			1,
			vk::ShaderStageFlagBits::eFragment,
			{});
		perRenderableBindings.push_back(materialSamplerBinding);
	}
	if (m_id.model.materialPart.info.bits.metallicRoughnessMap)
	{
		vk::DescriptorSetLayoutBinding materialSamplerBinding(bindings++,
			vk::DescriptorType::eCombinedImageSampler,
			1,
			vk::ShaderStageFlagBits::eFragment,
			{});
		perRenderableBindings.push_back(materialSamplerBinding);
	}
	if (perRenderableBindings.size() > 0)
	{
		descriptorSetLayouts.push_back(_createDescriptorSetLayout(perRenderableBindings));
	}

    if (m_id.model.primitivePart.info.bits.jointVertexData)
    {
        vk::DescriptorSetLayoutBinding animationUbo(bindings++,
            vk::DescriptorType::eUniformBuffer,
            1,
            vk::ShaderStageFlagBits::eVertex,
            {});
        descriptorSetLayouts.push_back(_createDescriptorSetLayout({ animationUbo }));
    }

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({ {},
		static_cast<uint32_t>(descriptorSetLayouts.size()),
		descriptorSetLayouts.data(),
		static_cast<uint32_t>(pushConstantRanges.size()),
		pushConstantRanges.data() });
	_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

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
		renderPass->Get(),
		{},
		{},
		static_cast<int32_t>(-1) });

	_graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

	for (auto descriptorSetLayout : descriptorSetLayouts)
	{
		m_device.destroyDescriptorSetLayout(descriptorSetLayout);
	}
}

void Pipeline::InitSkybox(std::shared_ptr<RenderPass> renderPass)
{
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    // set shader state
    vk::Device device = m_device;
    ShaderModule vertexShader(&device, m_id);
    vertexShader.BuildFromFile("Shaders/skybox.vert", ShaderStage::VERTEX, "main");

    ShaderModule fragmentShader(&device, m_id);
    fragmentShader.BuildFromFile("Shaders/skybox.frag", ShaderStage::FRAGMENT, "main");

    shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
    shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

    // set vertex input state
    // vertex
    _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
    _addAttributes(0, 0, vk::Format::eR32G32B32Sfloat, 0);

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
        static_cast<uint32_t>(_inputBindings.size()),
        _inputBindings.data(),
        static_cast<uint32_t>(_inputAttributes.size()),
        _inputAttributes.data() });

    // Set input assembly state
    vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
    assemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
    vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

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
        /* scissor.offset */{ 0, 0 },
        /* scissor.extent */{ width, height } };

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
        vk::CompareOp::eLessOrEqual,
        static_cast<vk::Bool32>(false),
        static_cast<vk::Bool32>(false),
        {},
        {},
        0.0f,
        1.0f });

    // TODO: Extract the descriptorSetlayout part as a single class.
    // descriptor set layout
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    // camera uniform buffer
    vk::DescriptorSetLayoutBinding cameraBinding(0,
        vk::DescriptorType::eUniformBuffer,
        1,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        {});
    descriptorSetLayouts.push_back(_createDescriptorSetLayout({ cameraBinding }));

    vk::DescriptorSetLayoutBinding skyboxSamplerBinding(0,
        vk::DescriptorType::eCombinedImageSampler,
        1,
        vk::ShaderStageFlagBits::eFragment,
        {}
    );
    descriptorSetLayouts.push_back(_createDescriptorSetLayout({ skyboxSamplerBinding }));
     
    // pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({},
        static_cast<uint32_t>(descriptorSetLayouts.size()),
        descriptorSetLayouts.data(),
        0,
        {});
    _pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

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
		renderPass->Get(),
        {},
        {},
        static_cast<int32_t>(-1) });

    _graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

    for (auto descriptorSetLayout : descriptorSetLayouts)
    {
        m_device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
}

void Pipeline::InitPrefilteredCubeMap(vk::Device device, vk::RenderPass renderPass)
{
    m_device = device;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    // set shader state
    ShaderModule vertexShader(&device, m_id);
    vertexShader.BuildFromFile("Shaders/prefilteredCubeMap.vert", ShaderStage::VERTEX, "main");

    ShaderModule fragmentShader(&device, m_id);
    fragmentShader.BuildFromFile("Shaders/prefilteredCubeMap.frag", ShaderStage::FRAGMENT, "main");

    shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
    shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

    // set vertex input state
    // vertex
    _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
    _addAttributes(0, 0, vk::Format::eR32G32B32Sfloat, 0);

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
        static_cast<uint32_t>(_inputBindings.size()),
        _inputBindings.data(),
        static_cast<uint32_t>(_inputAttributes.size()),
        _inputAttributes.data() });

    // Set input assembly state
    vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
    assemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
    vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

    uint32_t width = 1, height = 1;
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
        /* scissor.offset */{ 0, 0 },
        /* scissor.extent */{ width, height } };

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
        vk::FrontFace::eClockwise,
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
        vk::CompareOp::eLessOrEqual,
        static_cast<vk::Bool32>(false),
        static_cast<vk::Bool32>(false),
        {},
        {},
        0.0f,
        1.0f });

    // descriptor set layout
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

    vk::DescriptorSetLayoutBinding skyboxSamplerBinding({ 0,
        vk::DescriptorType::eCombinedImageSampler,
        1,
        vk::ShaderStageFlagBits::eFragment,
        {}
    });

    descriptorSetLayouts.push_back(_createDescriptorSetLayout({ skyboxSamplerBinding }));

    std::vector<vk::PushConstantRange> pushConstantRanges;

    uint32_t offset = 0;
    struct PushBlock {
        glm::mat4 mvp;
        float roughness;
        //uint32_t numSamples = 32u;
    };
    pushConstantRanges.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, offset, sizeof(PushBlock)));

    // pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({},
        static_cast<uint32_t>(descriptorSetLayouts.size()),
        descriptorSetLayouts.data(),
        static_cast<uint32_t>(pushConstantRanges.size()),
        pushConstantRanges.data());

    _pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

    std::array<vk::DynamicState, 2> dynamicStates;
    dynamicStates[0] = vk::DynamicState::eViewport;
    dynamicStates[1] = vk::DynamicState::eScissor;
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates.size(), dynamicStates.data());
    vk::GraphicsPipelineCreateInfo pipelineInfo({},
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
        &dynamicStateCreateInfo,
        _pipelineLayout,
        renderPass,
        {},
        {},
        static_cast<int32_t>(-1));

    _graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

    for (auto descriptorSetLayout : descriptorSetLayouts)
    {
        m_device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
}

void Pipeline::InitIrradianceMap(vk::Device device, vk::RenderPass renderPass)
{
    m_device = device;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    // set shader state
    ShaderModule vertexShader(&device, m_id);
    vertexShader.BuildFromFile("Shaders/prefilteredCubeMap.vert", ShaderStage::VERTEX, "main");

    ShaderModule fragmentShader(&device, m_id);
    fragmentShader.BuildFromFile("Shaders/irradianceMap.frag", ShaderStage::FRAGMENT, "main");

    shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
    shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

    // set vertex input state
    // vertex
    _addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
    _addAttributes(0, 0, vk::Format::eR32G32B32Sfloat, 0);

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
        static_cast<uint32_t>(_inputBindings.size()),
        _inputBindings.data(),
        static_cast<uint32_t>(_inputAttributes.size()),
        _inputAttributes.data() });

    // Set input assembly state
    vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
    assemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
    vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

    uint32_t width = 1, height = 1;
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
        /* scissor.offset */{ 0, 0 },
        /* scissor.extent */{ width, height } };

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
        vk::FrontFace::eClockwise,
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
        vk::CompareOp::eLessOrEqual,
        static_cast<vk::Bool32>(false),
        static_cast<vk::Bool32>(false),
        {},
        {},
        0.0f,
        1.0f });

    // descriptor set layout
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

    vk::DescriptorSetLayoutBinding skyboxSamplerBinding({ 0,
        vk::DescriptorType::eCombinedImageSampler,
        1,
        vk::ShaderStageFlagBits::eFragment,
        {}
    });

    descriptorSetLayouts.push_back(_createDescriptorSetLayout({ skyboxSamplerBinding }));

    std::vector<vk::PushConstantRange> pushConstantRanges;

    uint32_t offset = 0;
    struct PushBlock {
        glm::mat4 mvp;
        float roughness;
        //uint32_t numSamples = 32u;
    };
    pushConstantRanges.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, offset, sizeof(PushBlock)));

    // pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({},
        static_cast<uint32_t>(descriptorSetLayouts.size()),
        descriptorSetLayouts.data(),
        static_cast<uint32_t>(pushConstantRanges.size()),
        pushConstantRanges.data());

    _pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

    std::array<vk::DynamicState, 2> dynamicStates;
    dynamicStates[0] = vk::DynamicState::eViewport;
    dynamicStates[1] = vk::DynamicState::eScissor;
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates.size(), dynamicStates.data());
    vk::GraphicsPipelineCreateInfo pipelineInfo({},
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
        &dynamicStateCreateInfo,
        _pipelineLayout,
        renderPass,
        {},
        {},
        static_cast<int32_t>(-1));

    _graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

    for (auto descriptorSetLayout : descriptorSetLayouts)
    {
        m_device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
}

void Pipeline::InitGenerateBrdfLut(vk::Device device, vk::RenderPass renderPass)
{
    m_device = device;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    // set shader state
    ShaderModule vertexShader(&device, m_id);
    vertexShader.BuildFromFile("Shaders/quad.vert", ShaderStage::VERTEX, "main");

    ShaderModule fragmentShader(&device, m_id);
    fragmentShader.BuildFromFile("Shaders/genbrdflut.frag", ShaderStage::FRAGMENT, "main");

    shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
    shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

    // set vertex input state
    // vertex
    //_addInputBinding(sizeof(glm::vec3), vk::VertexInputRate::eVertex);
    //_addAttributes(0, 0, vk::Format::eR32G32B32Sfloat, 0);

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
        static_cast<uint32_t>(_inputBindings.size()),
        _inputBindings.data(),
        static_cast<uint32_t>(_inputAttributes.size()),
        _inputAttributes.data() });

    // Set input assembly state
    vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
    assemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
    vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

    uint32_t width = 1, height = 1;
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
        /* scissor.offset */{ 0, 0 },
        /* scissor.extent */{ width, height } };

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
        vk::FrontFace::eClockwise,
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
        vk::CompareOp::eLessOrEqual,
        static_cast<vk::Bool32>(false),
        static_cast<vk::Bool32>(false),
        {},
        {},
        0.0f,
        1.0f });

    // descriptor set layout
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

    std::vector<vk::PushConstantRange> pushConstantRanges;

    // pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({},
        static_cast<uint32_t>(descriptorSetLayouts.size()),
        descriptorSetLayouts.data(),
        static_cast<uint32_t>(pushConstantRanges.size()),
        pushConstantRanges.data());

    _pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

    std::array<vk::DynamicState, 2> dynamicStates;
    dynamicStates[0] = vk::DynamicState::eViewport;
    dynamicStates[1] = vk::DynamicState::eScissor;
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates.size(), dynamicStates.data());
    vk::GraphicsPipelineCreateInfo pipelineInfo({},
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
        &dynamicStateCreateInfo,
        _pipelineLayout,
        renderPass,
        {},
        {},
        static_cast<int32_t>(-1));

    _graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

    for (auto descriptorSetLayout : descriptorSetLayouts)
    {
        m_device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
}

void Pipeline::InitBrightPass(vk::Device device, vk::RenderPass renderPass)
{
	m_device = device;
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	// set shader state
	ShaderModule vertexShader(&device, m_id);
	vertexShader.BuildFromFile("Shaders/quad.vert", ShaderStage::VERTEX, "main");

	ShaderModule fragmentShader(&device, m_id);
	fragmentShader.BuildFromFile("Shaders/brightness.frag", ShaderStage::FRAGMENT, "main");

	shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
	shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
		static_cast<uint32_t>(_inputBindings.size()),
		_inputBindings.data(),
		static_cast<uint32_t>(_inputAttributes.size()),
		_inputAttributes.data() });

	// Set input assembly state
	vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
	assemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
	vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

	uint32_t width = 1, height = 1;
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
		/* scissor.offset */{ 0, 0 },
		/* scissor.extent */{ width, height } };

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
		vk::FrontFace::eClockwise,
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
		vk::CompareOp::eLessOrEqual,
		static_cast<vk::Bool32>(false),
		static_cast<vk::Bool32>(false),
		{},
		{},
		0.0f,
		1.0f });

	// descriptor set layout
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

	vk::DescriptorSetLayoutBinding samplerBinding({ 0,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	});

	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ samplerBinding }));

	std::vector<vk::PushConstantRange> pushConstantRanges;

	uint32_t offset = 0;
	uint32_t size = sizeof(float);
	pushConstantRanges.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment, offset, size));

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({},
		static_cast<uint32_t>(descriptorSetLayouts.size()),
		descriptorSetLayouts.data(),
		static_cast<uint32_t>(pushConstantRanges.size()),
		pushConstantRanges.data());

	_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

	std::array<vk::DynamicState, 2> dynamicStates;
	dynamicStates[0] = vk::DynamicState::eViewport;
	dynamicStates[1] = vk::DynamicState::eScissor;
	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates.size(), dynamicStates.data());
	vk::GraphicsPipelineCreateInfo pipelineInfo({},
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
		&dynamicStateCreateInfo,
		_pipelineLayout,
		renderPass,
		{},
		{},
		static_cast<int32_t>(-1));

	_graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

	for (auto descriptorSetLayout : descriptorSetLayouts)
	{
		m_device.destroyDescriptorSetLayout(descriptorSetLayout);
	}
}

void Pipeline::InitGaussianBlur(vk::Device device, vk::RenderPass renderPass)
{
	m_device = device;
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	// set shader state
	ShaderModule vertexShader(&device, m_id);
	vertexShader.BuildFromFile("Shaders/quad.vert", ShaderStage::VERTEX, "main");

	ShaderModule fragmentShader(&device, m_id);
	fragmentShader.BuildFromFile("Shaders/gaussianBlur.frag", ShaderStage::FRAGMENT, "main");

	shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
	shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());


	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
		static_cast<uint32_t>(_inputBindings.size()),
		_inputBindings.data(),
		static_cast<uint32_t>(_inputAttributes.size()),
		_inputAttributes.data() });

	// Set input assembly state
	vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
	assemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
	vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

	uint32_t width = 1, height = 1;
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
		/* scissor.offset */{ 0, 0 },
		/* scissor.extent */{ width, height } };

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
		vk::FrontFace::eClockwise,
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
		vk::CompareOp::eLessOrEqual,
		static_cast<vk::Bool32>(false),
		static_cast<vk::Bool32>(false),
		{},
		{},
		0.0f,
		1.0f });

	// descriptor set layout
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

	vk::DescriptorSetLayoutBinding samplerBinding(0,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{});
	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ samplerBinding }));

	std::vector<vk::PushConstantRange> pushConstantRanges;
	uint32_t offset = 0;
	uint32_t size = sizeof(float) * 4;
	pushConstantRanges.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment, offset, size));

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({},
		static_cast<uint32_t>(descriptorSetLayouts.size()),
		descriptorSetLayouts.data(),
		static_cast<uint32_t>(pushConstantRanges.size()),
		pushConstantRanges.data());

	_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

	std::array<vk::DynamicState, 2> dynamicStates;
	dynamicStates[0] = vk::DynamicState::eViewport;
	dynamicStates[1] = vk::DynamicState::eScissor;
	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates.size(), dynamicStates.data());
	vk::GraphicsPipelineCreateInfo pipelineInfo({},
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
		&dynamicStateCreateInfo,
		_pipelineLayout,
		renderPass,
		{},
		{},
		static_cast<int32_t>(-1));

	_graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

	for (auto descriptorSetLayout : descriptorSetLayouts)
	{
		m_device.destroyDescriptorSetLayout(descriptorSetLayout);
	}
}

void Pipeline::InitBlit(vk::Device device, vk::RenderPass renderPass)
{
	m_device = device;
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	// set shader state
	ShaderModule vertexShader(&device, m_id);
	vertexShader.BuildFromFile("Shaders/quad.vert", ShaderStage::VERTEX, "main");

	ShaderModule fragmentShader(&device, m_id);
	fragmentShader.BuildFromFile("Shaders/blit.frag", ShaderStage::FRAGMENT, "main");

	shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
	shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
		static_cast<uint32_t>(_inputBindings.size()),
		_inputBindings.data(),
		static_cast<uint32_t>(_inputAttributes.size()),
		_inputAttributes.data() });

	// Set input assembly state
	vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
	assemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
	vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

	uint32_t width = 1, height = 1;
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
		/* scissor.offset */{ 0, 0 },
		/* scissor.extent */{ width, height } };

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
		vk::FrontFace::eClockwise,
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
		vk::CompareOp::eLessOrEqual,
		static_cast<vk::Bool32>(false),
		static_cast<vk::Bool32>(false),
		{},
		{},
		0.0f,
		1.0f });

	// descriptor set layout
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

	vk::DescriptorSetLayoutBinding blurredSamplerBinding({ 0,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	});

	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ blurredSamplerBinding }));

	vk::DescriptorSetLayoutBinding originSamplerBinding({ 0,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	});

	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ originSamplerBinding }));

	std::vector<vk::PushConstantRange> pushConstantRanges;

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({},
		static_cast<uint32_t>(descriptorSetLayouts.size()),
		descriptorSetLayouts.data(),
		static_cast<uint32_t>(pushConstantRanges.size()),
		pushConstantRanges.data());

	_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

	std::array<vk::DynamicState, 2> dynamicStates;
	dynamicStates[0] = vk::DynamicState::eViewport;
	dynamicStates[1] = vk::DynamicState::eScissor;
	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates.size(), dynamicStates.data());
	vk::GraphicsPipelineCreateInfo pipelineInfo({},
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
		&dynamicStateCreateInfo,
		_pipelineLayout,
		renderPass,
		{},
		{},
		static_cast<int32_t>(-1));

	_graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

	for (auto descriptorSetLayout : descriptorSetLayouts)
	{
		m_device.destroyDescriptorSetLayout(descriptorSetLayout);
	}
}

void Pipeline::InitDeferred(std::shared_ptr<RenderPass> renderPass)
{
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	// set shader state
	ShaderModule vertexShader(&m_device, m_id);
	vertexShader.BuildFromFile("Shaders/quad.vert", ShaderStage::VERTEX, "main");

	ShaderModule fragmentShader(&m_device, m_id);
	fragmentShader.BuildFromFile("Shaders/deferred.frag", ShaderStage::FRAGMENT, "main");

	shaderStages.push_back(vertexShader.GetShaderStageCreateInfo());
	shaderStages.push_back(fragmentShader.GetShaderStageCreateInfo());

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({ {},
		static_cast<uint32_t>(_inputBindings.size()),
		_inputBindings.data(),
		static_cast<uint32_t>(_inputAttributes.size()),
		_inputAttributes.data() });

	// Set input assembly state
	vk::PipelineInputAssemblyStateCreateInfo assemblyInfo;
	assemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
	vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

	uint32_t width = 1, height = 1;
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
		/* scissor.offset */{ 0, 0 },
		/* scissor.extent */{ width, height } };

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
		vk::FrontFace::eClockwise,
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
		static_cast<vk::Bool32>(false),
		vk::CompareOp::eLessOrEqual,
		static_cast<vk::Bool32>(false),
		static_cast<vk::Bool32>(false),
		{},
		{},
		0.0f,
		1.0f });

	// descriptor set layout
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

	vk::DescriptorSetLayoutBinding cameraUniformBinding(0,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		{}
	);

	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ cameraUniformBinding }));

	vk::DescriptorSetLayoutBinding lightUniformBinding(0,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	);

	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ lightUniformBinding }));

	vk::DescriptorSetLayoutBinding preFilteredCubeMapBinding(0,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	);

	vk::DescriptorSetLayoutBinding irradianceMapBinding(1,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	);

	vk::DescriptorSetLayoutBinding brdfBinding(2,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	);

	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ preFilteredCubeMapBinding, irradianceMapBinding, brdfBinding }));

	vk::DescriptorSetLayoutBinding positionSamplerBinding(0,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	);

	vk::DescriptorSetLayoutBinding normalSamplerBinding(1,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	);

	vk::DescriptorSetLayoutBinding albedoSamplerBinding(2,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	);

	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ positionSamplerBinding, normalSamplerBinding, albedoSamplerBinding }));


	vk::DescriptorSetLayoutBinding shadowMapSamplerBinding(0,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		{}
	);
	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ shadowMapSamplerBinding }));

	vk::DescriptorSetLayoutBinding shadowMapUniformBinding(0,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		{});
	descriptorSetLayouts.push_back(_createDescriptorSetLayout({ shadowMapUniformBinding }));


	std::vector<vk::PushConstantRange> pushConstantRanges;

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({},
		static_cast<uint32_t>(descriptorSetLayouts.size()),
		descriptorSetLayouts.data(),
		static_cast<uint32_t>(pushConstantRanges.size()),
		pushConstantRanges.data());

	_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

	std::array<vk::DynamicState, 2> dynamicStates;
	dynamicStates[0] = vk::DynamicState::eViewport;
	dynamicStates[1] = vk::DynamicState::eScissor;
	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates.size(), dynamicStates.data());
	vk::GraphicsPipelineCreateInfo pipelineInfo({},
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
		&dynamicStateCreateInfo,
		_pipelineLayout,
		renderPass->Get(),
		{},
		{},
		static_cast<int32_t>(-1));

	_graphicsPipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;

	for (auto descriptorSetLayout : descriptorSetLayouts)
	{
		m_device.destroyDescriptorSetLayout(descriptorSetLayout);
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
