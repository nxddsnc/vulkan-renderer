#include "MyScene.h"
#include "MyAnimation.h"

MyScene::MyScene()
{
	m_bbox.min = glm::vec3(INFINITE);
	m_bbox.max = -glm::vec3(INFINITE);
}


MyScene::~MyScene()
{
}

void MyScene::AddDrawable(std::shared_ptr<Drawable> node)
{
    _drawables.push_back(node);

	if (m_bbox.min.x > node->m_bbox.min.x)
	{
		m_bbox.min.x = node->m_bbox.min.x;
	}
	if (m_bbox.min.y > node->m_bbox.min.y)
	{
		m_bbox.min.y = node->m_bbox.min.y;
	}
	if (m_bbox.min.z > node->m_bbox.min.z)
	{
		m_bbox.min.z = node->m_bbox.min.z;
	}

	if (m_bbox.max.x < node->m_bbox.max.x)
	{
		m_bbox.max.x = node->m_bbox.max.x;
	}
	if (m_bbox.max.y < node->m_bbox.max.y)
	{
		m_bbox.max.y = node->m_bbox.max.y;
	}
	if (m_bbox.max.z < node->m_bbox.max.z)
	{
		m_bbox.max.z = node->m_bbox.max.z;
	}
}

std::vector<std::shared_ptr<Drawable>> MyScene::GetDrawables()
{
    return _drawables;
}

void MyScene::AddAnimation(std::shared_ptr<MyAnimation> animation)
{
	m_animations.push_back(animation);
}
