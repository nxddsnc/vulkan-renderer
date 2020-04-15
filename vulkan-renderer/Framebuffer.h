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
	Framebuffer(const char* name, ResourceManager* resourceManager, std::vector<MyImageFormat> colorFormats, MyImageFormat depthFormt, int width, int height);
	Framebuffer(const char* name, ResourceManager* resourceManager, std::shared_ptr<RenderPass> renderPass, 
		std::shared_ptr<VulkanTexture> colorTexture, std::shared_ptr<VulkanTexture> depthStencilTexture, int width, int height);
	~Framebuffer();

public:
	
	std::shared_ptr<RenderPass>					m_pRenderPass;
	vk::Framebuffer								m_vkFramebuffer;
	vk::DescriptorSet							m_dsTexture;

	std::vector<std::shared_ptr<VulkanTexture>> m_pColorTextures;
	std::shared_ptr<VulkanTexture>				m_pDepthTexture;
private:
	ResourceManager							  * m_pResourceManager;

	char										m_pName[512];
	
private:
	void _init(std::vector<MyImageFormat> colorFormats, MyImageFormat depthFormt, int width, int height);
	void _createVulkanFramebuffer(int width, int height);
	void _deInit();
};

