#include "Framebuffer.h"
#include "RenderPass.h"
#include "ResourceManager.h"
#include "Drawable.h"
#include "MyTexture.h"

Framebuffer::Framebuffer()
{
}

Framebuffer::Framebuffer(const char* name, ResourceManager* resourceManager, MyImageFormat colorFormat, MyImageFormat depthFormt, int width, int height)
{
	m_pResourceManager = resourceManager;

	sprintf(m_pName, "%s", name);

	m_pColorTexture = nullptr;
	m_pDepthTexture = nullptr;
	_init(colorFormat, depthFormt, width, height);
}

Framebuffer::~Framebuffer()
{
	_deInit();
}

void Framebuffer::_init(MyImageFormat colorFormat, MyImageFormat depthFormt, int width, int height)
{
	std::vector<vk::ImageView> attachments;
	m_pRenderPass = std::make_shared<RenderPass>(&(m_pResourceManager->m_device));

	std::shared_ptr<MyTexture> colorTexture = std::make_shared<MyTexture>();
	char colorBufferName[512];
	sprintf(colorBufferName, "%s-%s", m_pName, "color");
	colorTexture->m_pImage = std::make_shared<MyImage>(colorBufferName, width, height, colorFormat, true);

	m_pColorTexture = m_pResourceManager->CreateCombinedTexture(colorTexture);

	vk::Format cFormat = vk::Format::eR8G8B8A8Unorm;
	switch (colorFormat)
	{
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

	attachments.push_back(m_pColorTexture->imageView);

	if (depthFormt != MY_IMAGEFORMAT_UNKONWN)
	{
		std::shared_ptr<MyTexture> depthTexture = std::make_shared<MyTexture>();
		char depthBufferName[512];
		sprintf(depthBufferName, "%s-%s", m_pName, "depth");
		depthTexture->m_pImage = std::make_shared<MyImage>(depthBufferName, width, height, depthFormt, true);
		m_pDepthTexture = m_pResourceManager->CreateCombinedTexture(depthTexture);

		vk::Format dFormat = vk::Format::eD24UnormS8Uint;
		switch (m_pDepthTexture->texture->m_pImage->m_format)
		{
		case MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT:
			dFormat = vk::Format::eD24UnormS8Uint;
			break;
		}
		vk::AttachmentDescription depthAttachment({
			{},
			dFormat,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eStore,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		});
		m_pRenderPass->AddAttachment(depthAttachment);
	
		attachments.push_back(m_pDepthTexture->imageView);
	}

	vk::FramebufferCreateInfo createInfo({ {},
		m_pRenderPass->Get(),
		(uint32_t)attachments.size(),
		attachments.data(),
		m_pColorTexture->texture->m_pImage->m_width,
		m_pColorTexture->texture->m_pImage->m_height,
		uint32_t(1) });

	m_vkFramebuffer = m_pResourceManager->m_device.createFramebuffer(createInfo);

	// Create descriptor set.
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

void Framebuffer::_deInit()
{
	m_pRenderPass = nullptr;
	m_pResourceManager->m_device.destroyFramebuffer(m_vkFramebuffer);
	
}