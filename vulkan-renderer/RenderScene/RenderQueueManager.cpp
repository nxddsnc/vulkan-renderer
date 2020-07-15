#include "RenderQueueManager.h"
#include "Pipeline.h"
#include "PipelineManager.h"
#include "RenderQueue.h"

RenderQueueManager::RenderQueueManager(PipelineManager *pipelineMananger)
{
	m_pPipelineManager = pipelineMananger;
}


RenderQueueManager::~RenderQueueManager()
{
}

bool RenderQueueManager::HasRenderQueue(PipelineId pipelineId)
{
	return m_renderQueueMap.count(pipelineId) > 0;
}

std::shared_ptr<RenderQueue> RenderQueueManager::GetRenderQueue(PipelineId pipelineId, std::shared_ptr<RenderPass> renderPass)
{
	if (m_renderQueueMap.count(pipelineId) > 0)
	{
		return m_renderQueueMap.at(pipelineId);
	}
	else
	{
		std::shared_ptr<Pipeline>  pipeline = m_pPipelineManager->GetPipeline(pipelineId, renderPass);
		std::shared_ptr<RenderQueue> renderQueue = std::make_shared<RenderQueue>(pipeline);
		m_renderQueueMap.insert(std::make_pair(pipelineId, renderQueue));

		m_renderQueues.push_back(renderQueue);

		return renderQueue;
	}
}
