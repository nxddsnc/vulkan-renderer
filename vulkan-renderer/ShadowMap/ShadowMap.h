#include "Platform.h"
#include "Utils.h"
class Framebuffer;
class MyCamera;
class ResourceManager;
class PipelineManager;
class RenderScene;
class RenderQueue;
class RenderQueueManager;
class MyScene;
class ShadowMap
{
public:
	ShadowMap(ResourceManager *resourceManager, PipelineManager *pipelineManager, std::shared_ptr<RenderQueueManager> renderQueueManager);
	~ShadowMap();

	std::shared_ptr<Framebuffer> GetFramebuffer();

	void AddScene(std::shared_ptr<MyScene> scene);

	std::shared_ptr<Framebuffer> Draw(vk::CommandBuffer& commandBuffer);
	std::shared_ptr<MyCamera>				m_pCamera;
	std::shared_ptr<Framebuffer>			m_pFramebuffer;
private:

	BBox									m_bbox;

	int										m_width;
	int										m_height;

	ResourceManager						*   m_pResourceManager;
	PipelineManager					    *   m_pPipelineManager;

	std::shared_ptr<RenderScene>			m_pRenderScene;

	std::shared_ptr<RenderQueueManager>         m_pRenderQueuemanager;
	std::vector < std::shared_ptr<RenderQueue>> m_renderQueues;


	glm::vec3								m_lightDir;

	void _init();
	void _deInit();

	void _update();
};