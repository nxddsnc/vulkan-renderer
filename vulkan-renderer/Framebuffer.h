#include "Platform.h"
#include "MyImage.h"
#pragma once

struct AttachmentInfo
{
	std::shared_ptr<MyImageFormat> format;
	int						       uniqueId;

};
class ResourceManager;
struct VulkanTexture;
class RenderPass;
class Framebuffer
{
public:
	Framebuffer();
	Framebuffer(const char* name, ResourceManager* resourceManager, std::vector<MyImageFormat> colorFormats, MyImageFormat depthFormt, int width, int height, bool depthAsSampler = false);
	Framebuffer(const char* colorName, const char* depthStencilName, ResourceManager* resourceManager, std::vector<MyImageFormat> colorFormats, MyImageFormat depthFormt, int width, int height);
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

	char										m_pColorName[512];
	char										m_pDepthStencilName[512];
	bool										m_bDepthAsSampler;
private:
	void _init(std::vector<MyImageFormat> colorFormats, MyImageFormat depthFormt, int width, int height);
	void _createVulkanFramebuffer(int width, int height);
	void _deInit();
};

