#include "Framebuffer.h"
#include "RenderPass.h"
#include "ResourceManager.h"
#include "Drawable.h"
#include "MyTexture.h"

Framebuffer::Framebuffer(ResourceManager* resourceManager, vk::DescriptorPool descritptorPool, std::shared_ptr<VulkanTexture> colorTexture, std::shared_ptr<VulkanTexture> depthTexture)
{
	m_pResourceManager = resourceManager;
	m_descriptorPool = descritptorPool;
	m_pColorTexture = colorTexture;
	m_pDepthTexture = depthTexture;

	_init();
}

Framebuffer::~Framebuffer()
{
	_deInit();
}

void Framebuffer::_init()
{
	vk::Format format = vk::Format::eR8G8B8A8Unorm;
	switch (m_pColorTexture->texture->m_pImage->m_format)
	{
	case MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT:
		format = vk::Format::eR16G16B16A16Sfloat;
		break;
	case MyImageFormat::MY_IMAGEFORMAT_RGBA32_FLOAT:
		format = vk::Format::eR32G32B32A32Sfloat;
		break;
	}
	m_pRenderPass = std::make_shared<RenderPass>(&(m_pResourceManager->m_device));
	vk::AttachmentDescription colorAttachment({
		{},
		format,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal
	});

	vk::Format depthFormat = vk::Format::eD24UnormS8Uint;
	switch (m_pDepthTexture->texture->m_pImage->m_format)
	{
	case MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT:
		depthFormat = vk::Format::eD24UnormS8Uint;
		break;
	}
	vk::AttachmentDescription depthAttachment({
		{},
		depthFormat,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eStore,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	});
	m_pRenderPass->AddAttachment(colorAttachment);
	m_pRenderPass->AddAttachment(depthAttachment);

	std::array<vk::ImageView, 2> attachments{};
	attachments[0] = m_pColorTexture->imageView;
	attachments[1] = m_pDepthTexture->imageView;

	vk::FramebufferCreateInfo createInfo({ {},
		m_pRenderPass->Get(),
		(uint32_t)attachments.size(),
		attachments.data(),
		m_pColorTexture->texture->m_pImage->m_width,
		m_pColorTexture->texture->m_pImage->m_height,
		uint32_t(1) });

	m_vkFramebuffer = m_pResourceManager->m_device.createFramebuffer(createInfo);


	{
		std::vector<vk::DescriptorSetLayoutBinding> textureBindings;
		std::vector<vk::DescriptorImageInfo> imageInfos;
		vk::DescriptorSetLayoutBinding textureBinding(0,
			vk::DescriptorType::eCombinedImageSampler,
			1,
			vk::ShaderStageFlagBits::eFragment,
			{});
		textureBindings.push_back(textureBinding);

		vk::DescriptorImageInfo imageInfo(m_pColorTexture->imageSampler,
			m_pColorTexture->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
		imageInfos.push_back(imageInfo);

		vk::DescriptorSetLayout descriptorSetLayout;

		vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32_t>(textureBindings.size()), textureBindings.data());
		descriptorSetLayout = m_pResourceManager->m_device.createDescriptorSetLayout(layoutInfo);

		vk::DescriptorSetAllocateInfo allocInfo(m_descriptorPool, 1, &descriptorSetLayout);

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

void Framebuffer::_deInit()
{
	m_pRenderPass = nullptr;
	m_pResourceManager->m_device.destroyFramebuffer(m_vkFramebuffer);
	
}