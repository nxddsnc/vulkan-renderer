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

void MyScene::AddDrawable(std::shared_ptr<Drawable> node)
{
    _drawables.push_back(node);
	m_bbox.Merge(node->m_bbox);
}

std::vector<std::shared_ptr<Drawable>> MyScene::GetDrawables()
{
	if (_instanceComputed)
	{
		return _drawables;
	}
	size_t len = _drawables.size();
	std::vector<std::shared_ptr<Drawable>> res;
	for (int i = 0; i < len; ++i)
	{
		auto drawable = _drawables[i];
		std::shared_ptr<Drawable> d;
		auto it = _drawableMap.find(drawable->GetHash());
		if (it != _drawableMap.end())
		{
			d = it->second;
			if (d->m_type == SINGLE_DRAWABLE)
			{
				auto instanceDrawable = std::make_shared<InstanceDrawable>(std::dynamic_pointer_cast<SingleDrawable>(d));
				instanceDrawable->AddDrawable(std::dynamic_pointer_cast<SingleDrawable>(drawable));
				it->second = instanceDrawable;

			}
			else
			{
				std::shared_ptr<InstanceDrawable> d_ = std::dynamic_pointer_cast<InstanceDrawable>(d);
				d_->AddDrawable(std::dynamic_pointer_cast<SingleDrawable>(drawable));
			}
		}
		else 
		{
			_drawableMap.insert(std::make_pair(drawable->GetHash(), drawable));
		}
	}
	for (auto it = _drawableMap.begin(); it != _drawableMap.end(); ++it)
	{
		res.emplace_back(it->second);
	}
	_drawables = res;
	_instanceComputed = true;
	return _drawables;
}

void MyScene::AddAnimation(std::shared_ptr<MyAnimation> animation)
{
	m_animations.push_back(animation);
}
