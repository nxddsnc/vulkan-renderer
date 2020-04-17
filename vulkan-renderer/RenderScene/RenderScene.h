#include "Platform.h"
#include <unordered_map>
#include "Pipeline.h"
#include "Drawable.h"
#pragma once

class Drawable;
class Skybox;
class Axis;
class ResourceManager;
class PipelineManager;
class RenderQueue;
class RenderQueueManager;
class VulkanCamera;
class Framebuffer;
class MyScene;
class RenderScene
{
public:
	RenderScene(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height);
	virtual ~RenderScene();

	void AddScene(std::shared_ptr<MyScene> scene);
	virtual void Draw(vk::CommandBuffer& commandBuffer);
	void UpdateUniforms();

	virtual std::shared_ptr<Framebuffer> GetFramebuffer();

public:
	std::shared_ptr<Skybox>												m_pSkybox;
	std::shared_ptr<Axis>											    m_pAxis;
	std::shared_ptr<VulkanCamera>										m_pCamera;
	BBox																m_bbox;
protected:
	ResourceManager													  * m_pResourceManager;
	PipelineManager													  * m_pPipelineManager;
	vk::RenderPass														m_renderPass;

	int																	m_width;
	int																	m_height;

	std::shared_ptr<RenderQueueManager>									m_pRenderQueueManager;
	std::unordered_map<PipelineId, std::shared_ptr<RenderQueue>>        m_renderQueues;

	std::vector<std::shared_ptr<Framebuffer>>						    m_framebuffers;

	PipelineType														m_pipelineType;
private:
	virtual void _begin(vk::CommandBuffer &commandBuffer);
	virtual void _end(vk::CommandBuffer &commandBuffer);

	void _updateBBox();
};
