#include "Platform.h"
#pragma once

class ResourceManager;
struct VulkanTexture;
class RenderPass;
class Framebuffer
{
public:
	Framebuffer(ResourceManager* resourceManager, vk::DescriptorPool descritptorPool, std::shared_ptr<VulkanTexture> colorTexture, std::shared_ptr<VulkanTexture> depthTexture);
	~Framebuffer();

public:
	
	std::shared_ptr<RenderPass>		   m_pRenderPass;
	vk::Framebuffer					   m_vkFramebuffer;
	vk::DescriptorSet				   m_dsTexture;

	std::shared_ptr<VulkanTexture>     m_pColorTexture;
	std::shared_ptr<VulkanTexture>     m_pDepthTexture;
private:
	ResourceManager			          *m_pResourceManager;
	vk::DescriptorPool				   m_descriptorPool;
	
private:
	void _init();
	void _deInit();
};

