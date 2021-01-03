#include "MyScene.h"
#include "MyAnimation.h"

MyScene::MyScene()
{
	m_bbox.min = glm::vec3(INFINITE);
	m_bbox.max = -glm::vec3(INFINITE);
	_instanceComputed = false;
}

MyScene::~MyScene()
{
}

void MyScene::AddRenderable(std::shared_ptr<Renderable> node)
{
    _renderables.push_back(node);
	m_bbox.Merge(node->m_bbox);
}

std::vector<std::shared_ptr<Renderable>> MyScene::GetRenderables()
{
	if (_instanceComputed)
	{
		return _renderables;
	}
	size_t len = _renderables.size();
	std::vector<std::shared_ptr<Renderable>> res;
	for (int i = 0; i < len; ++i)
	{
		auto renderable = _renderables[i];
		std::shared_ptr<Renderable> d;
		auto it = _renderableMap.find(renderable->GetHash());
		if (it != _renderableMap.end())
		{
			d = it->second;
			if (d->m_type == SINGLE_RENDERABLE)
			{
				auto instanceRenderable = std::make_shared<InstanceRenderable>(std::dynamic_pointer_cast<SingleRenderable>(d));
				instanceRenderable->AddRenderable(std::dynamic_pointer_cast<SingleRenderable>(renderable));
				it->second = instanceRenderable;

			}
			else
			{
				std::shared_ptr<InstanceRenderable> d_ = std::dynamic_pointer_cast<InstanceRenderable>(d);
				d_->AddRenderable(std::dynamic_pointer_cast<SingleRenderable>(renderable));
			}
		}
		else
		{
			_renderableMap.insert(std::make_pair(renderable->GetHash(), renderable));
		}
	}
	for (auto it = _renderableMap.begin(); it != _renderableMap.end(); ++it)
	{
		res.emplace_back(it->second);
	}
	_renderables = res;
	_instanceComputed = true;
	return _renderables;
}

void MyScene::AddAnimation(std::shared_ptr<MyAnimation> animation)
{
	m_animations.push_back(animation);
}
