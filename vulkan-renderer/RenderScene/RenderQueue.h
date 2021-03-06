#include "Platform.h"

#pragma once
class Renderable;
class Pipeline;
class MyCamera;
class Skybox;
class ShadowMap;
class RenderQueue
{
public:
	RenderQueue(std::shared_ptr<Pipeline> pipeline);
	~RenderQueue();

	void AddRenderable(std::shared_ptr<Renderable> renderable);
	void Draw(vk::CommandBuffer commandBuffer, std::shared_ptr<MyCamera> camera, 
		std::shared_ptr<Skybox> skybox = nullptr, std::shared_ptr<ShadowMap> shadowMap = nullptr, int width = 0, int height = 0);
private:
	std::vector<std::shared_ptr<Renderable>>			m_renderables;
	std::shared_ptr<Pipeline>						m_pPipeline;
};

