#include "Platform.h"
#include <unordered_map>
#include "Pipeline.h"
#include "Drawable.h"
#pragma once

class SingleDrawable;
class Skybox;
class Axis;
class ResourceManager;
class PipelineManager;
class RenderQueue;
class RenderQueueManager;
class MyCamera;
class Framebuffer;
class MyScene;
class ShadowMap;
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
	std::shared_ptr<MyCamera>											m_pCamera;
	BBox																m_bbox;
	std::shared_ptr<ShadowMap>											m_pShadowMap;
    std::shared_ptr<MyAnimation>                                        m_animation;
protected:
	ResourceManager													  * m_pResourceManager;
	PipelineManager													  * m_pPipelineManager;
	vk::RenderPass														m_renderPass;

	int																	m_width;
	int																	m_height;

	std::shared_ptr<RenderQueueManager>									m_pRenderQueueManager;
	std::vector<std::shared_ptr<RenderQueue>>							m_renderQueues;

	std::vector<std::shared_ptr<Framebuffer>>						    m_framebuffers;


	PipelineType														m_pipelineType;
private:
	virtual void _begin(vk::CommandBuffer &commandBuffer);
	virtual void _end(vk::CommandBuffer &commandBuffer);

	void _updateBBox();
};
