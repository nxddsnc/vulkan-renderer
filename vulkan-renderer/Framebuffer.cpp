#include "Framebuffer.h"
#include "RenderPass.h"
#include "ResourceManager.h"
#include "Drawable.h"
#include "MyTexture.h"

Framebuffer::Framebuffer()
{
}

Framebuffer::Framebuffer(const char * name, ResourceManager * resourceManager, std::vector<MyImageFormat> colorFormats, MyImageFormat depthFormt, int width, int height, bool depthAsSampler)
{

	m_pResourceManager = resourceManager;
	m_bDepthAsSampler = depthAsSampler;

	sprintf(m_pColorName, "%s", name);
	sprintf(m_pDepthStencilName, "%s", name);

	m_pDepthTexture = nullptr;
	_init(colorFormats, depthFormt, width, height);
}

Framebuffer::Framebuffer(const char * colorName, const char * depthStencilName, ResourceManager * resourceManager, std::vector<MyImageFormat> colorFormats, MyImageFormat depthFormt, int width, int height)
{
	m_pResourceManager = resourceManager;
	m_bDepthAsSampler = false;
	sprintf(m_pColorName, "%s", colorName);
	sprintf(m_pDepthStencilName, "%s", depthStencilName);

	m_pDepthTexture = nullptr;
	_init(colorFormats, depthFormt, width, height);
}

Framebuffer::Framebuffer(const char * name, ResourceManager * resourceManager, std::shared_ptr<RenderPass> renderPass, 
	std::shared_ptr<VulkanTexture> colorTexture, std::shared_ptr<VulkanTexture> depthStencilTexture, int width, int height)
{
	sprintf(m_pColorName, "%s", name);
	m_bDepthAsSampler = false;
	m_pColorTextures.push_back(colorTexture);
	m_pDepthTexture = depthStencilTexture;

	m_pResourceManager = resourceManager;

	m_pRenderPass = renderPass;

	_createVulkanFramebuffer(width, height);
}

Framebuffer::~Framebuffer()
{
	_deInit();
}

void Framebuffer::_createVulkanFramebuffer(int width, int height)
{
	std::vector<vk::ImageView> attachments;
	for (int i = 0; i < m_pColorTextures.size(); ++i)
	{
		attachments.push_back(m_pColorTextures[i]->imageView);
	}
	if (m_pDepthTexture)
	{
		attachments.push_back(m_pDepthTexture->imageView);
	}

	vk::FramebufferCreateInfo createInfo({ {},
		m_pRenderPass->Get(),
		(uint32_t)attachments.size(),
		attachments.data(),
		(uint32_t)width,
		(uint32_t)height,
		uint32_t(1) });

	m_vkFramebuffer = m_pResourceManager->m_device.createFramebuffer(createInfo);

	// Create descriptor set.
	if (m_pColorTextures[0]->imageSampler)
	{
		std::vector<vk::DescriptorSetLayoutBinding> textureBindings;
		std::vector<vk::DescriptorImageInfo> imageInfos;
		
		int binding = 0;
		for (binding = 0; binding < m_pColorTextures.size(); ++binding)
		{
			vk::DescriptorSetLayoutBinding textureBinding(binding,
				vk::DescriptorType::eCombinedImageSampler,
				1,
				vk::ShaderStageFlagBits::eFragment,
				{});
			textureBindings.push_back(textureBinding);

			vk::DescriptorImageInfo imageInfo(m_pColorTextures[binding]->imageSampler,
				m_pColorTextures[binding]->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
			imageInfos.push_back(imageInfo);
		}

		if (m_pDepthTexture && m_pDepthTexture->imageSampler && m_bDepthAsSampler)
		{
			vk::DescriptorSetLayoutBinding textureBinding(binding,
				vk::DescriptorType::eCombinedImageSampler,
				1,
				vk::ShaderStageFlagBits::eFragment,
				{});
			textureBindings.push_back(textureBinding);

			vk::DescriptorImageInfo imageInfo(m_pDepthTexture->imageSampler,
				m_pDepthTexture->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
			imageInfos.push_back(imageInfo);
		}

		vk::DescriptorSetLayout descriptorSetLayout;

		vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32_t>(textureBindings.size()), textureBindings.data());
		descriptorSetLayout = m_pResourceManager->m_device.createDescriptorSetLayout(layoutInfo);

		vk::DescriptorSetAllocateInfo allocInfo(m_pResourceManager->m_descriptorPool, 1, &descriptorSetLayout);

		m_pResourceManager->m_device.allocateDescriptorSets(&allocInfo, &m_dsTexture);

		vk::WriteDescriptorSet descriptorWrite(m_dsTexture,
			uint32_t(0),
			uint32_t(0),
			static_cast<uint32_t>(imageInfos.size()),
			vk::DescriptorType::eCombinedImageSampler,
			imageInfos.data(),
			{},
			{});
		m_pResourceManager->m_device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
		m_pResourceManager->m_device.destroyDescriptorSetLayout(descriptorSetLayout);
	}
}

void Framebuffer::_init(std::vector<MyImageFormat> colorFormats, MyImageFormat depthFormt, int width, int height)
{
	m_pRenderPass = std::make_shared<RenderPass>(&(m_pResourceManager->m_device));

	for (int i = 0; i < colorFormats.size(); ++i)
	{
		std::shared_ptr<MyTexture> colorTexture = std::make_shared<MyTexture>();
		char colorBufferName[512];
		sprintf(colorBufferName, "%s-%s-%d", m_pColorName, "color", i);
		colorTexture->m_pImage = std::make_shared<MyImage>(colorBufferName, width, height, colorFormats[i], true);

		m_pColorTextures.push_back(m_pResourceManager->CreateCombinedTexture(colorTexture));

		vk::Format cFormat = vk::Format::eR8G8B8A8Unorm;
		switch (colorFormats[i])
		{
		case MyImageFormat::MY_IMAGEFORMAT_R16_FLOAT:
			cFormat = vk::Format::eR16Sfloat;
			break;
		case MyImageFormat::MY_IMAGEFORMAT_R32_FLOAT:
			cFormat = vk::Format::eR32Sfloat;
			break;
		case MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT:
			cFormat = vk::Format::eR16G16B16A16Sfloat;
			break;
		case MyImageFormat::MY_IMAGEFORMAT_RGBA32_FLOAT:
			cFormat = vk::Format::eR32G32B32A32Sfloat;
			break;
		}
		vk::AttachmentDescription colorAttachment({
			{},
			cFormat,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal
		});
		m_pRenderPass->AddAttachment(colorAttachment);
	}

	if (depthFormt != MY_IMAGEFORMAT_UNKONWN)
	{
		std::shared_ptr<MyTexture> depthTexture = std::make_shared<MyTexture>();
		char depthBufferName[512];
		sprintf(depthBufferName, "%s-%s", m_pDepthStencilName, "depth");
		depthTexture->m_pImage = std::make_shared<MyImage>(depthBufferName, width, height, depthFormt, true);
		m_pDepthTexture = m_pResourceManager->CreateCombinedTexture(depthTexture);

		vk::Format dFormat = vk::Format::eD24UnormS8Uint;
		switch (m_pDepthTexture->texture->m_pImage->m_format)
		{
		case MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT:
			dFormat = vk::Format::eD24UnormS8Uint;
			break;
		case MyImageFormat::MY_IMAGEFORMAT_D32_FLOAT:
			dFormat = vk::Format::eD32Sfloat;
			break;
		}
		vk::AttachmentDescription depthAttachment({
			{},
			dFormat,
			vk::SampleCountFlagBits::e1,
			m_pDepthTexture->referenceCount > 1 ? vk::AttachmentLoadOp::eDontCare : vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eStore,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		});
		m_pRenderPass->AddAttachment(depthAttachment);
	}

	_createVulkanFramebuffer(width, height);
}

void Framebuffer::_deInit()
{
	m_pRenderPass = nullptr;
	m_pResourceManager->m_device.destroyFramebuffer(m_vkFramebuffer);
}