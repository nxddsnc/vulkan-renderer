//#include "Platform.h"
//#include <unordered_map>
//#include "Pipeline.h"
//#pragma once
//
//class Drawable;
//class Skybox;
//class Axis;
//class ResourceManager;
//class PipelineManager;
//class RenderQueue;
//class RenderQueueManager;
//class VulkanCamera;
//class RenderScene
//{
//public:
//	RenderScene(ResourceManager *resourceManager, PipelineManager *pipelineManager);
//	~RenderScene();
//
//	void AddRenderNodes(std::vector<std::shared_ptr<Drawable>> drawables);
//	void Draw(vk::CommandBuffer& commandBuffer);
//
//	void UpdateUniforms();
//
//public:
//	std::shared_ptr<Skybox>												m_pSkybox;
//	std::shared_ptr<Axis>											    m_pAxis;
//	std::shared_ptr<VulkanCamera>										m_pCamera;
//
//private:
//	std::unordered_map<PipelineId, std::shared_ptr<Drawable>>           m_drawableMap;
//	std::vector<std::shared_ptr<Drawable>>								m_drawables;
//	ResourceManager													  * m_pResourceManager;
//	PipelineManager													  * m_pPipelineManager;
//	vk::RenderPass														m_renderPass;
//
//	std::shared_ptr<RenderQueueManager>									m_pRenderQueueManager;
//	std::unordered_map<PipelineId, std::shared_ptr<RenderQueue>>        m_renderQueues;
//};
