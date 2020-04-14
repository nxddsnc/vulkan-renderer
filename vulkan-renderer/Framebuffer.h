#include "Platform.h"
#include "MyImage.h"
#pragma once

class ResourceManager;
struct VulkanTexture;
class RenderPass;
class Framebuffer
{
public:
	Framebuffer();
	Framebuffer(const char* name, ResourceManager* resourceManager, MyImageFormat colorFormat, MyImageFormat depthFormt, int width, int height);
	~Framebuffer();

public:
	
	std::shared_ptr<RenderPass>		   m_pRenderPass;
	vk::Framebuffer					   m_vkFramebuffer;
	vk::DescriptorSet				   m_dsTexture;

	std::shared_ptr<VulkanTexture>     m_pColorTexture;
	std::shared_ptr<VulkanTexture>     m_pDepthTexture;
private:
	ResourceManager			          *m_pResourceManager;

	char							   m_pName[512];
	
private:
	void _init(MyImageFormat colorFormat, MyImageFormat depthFormt, int width, int height);
	void _deInit();
};

