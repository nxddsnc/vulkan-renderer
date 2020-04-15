#include "Pipeline.h"
#include <unordered_map>

#pragma once

class RenderQueue;
class PipelineManager;
class RenderQueueManager
{
public:
	RenderQueueManager(PipelineManager *pipelineManager);
	~RenderQueueManager();

	std::shared_ptr<RenderQueue> GetRenderQueue(PipelineId pipelineId);

	std::vector<std::shared_ptr<RenderQueue>>							m_renderQueues;
private:
	std::unordered_map<PipelineId, std::shared_ptr<RenderQueue>>		m_renderQueueMap;
	PipelineManager												      * m_pPipelineManager;
};

