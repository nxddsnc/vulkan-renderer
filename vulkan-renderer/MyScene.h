#include <glm/glm.hpp>
#include "Platform.h"
#include <array>
#include <vector>
#include "Drawable.h"
#include <memory.h>
#pragma once

class MyAnimation;
class MyScene
{
public:
	MyScene();
	~MyScene();

    void AddDrawable(std::shared_ptr<Drawable> node);
    std::vector<std::shared_ptr<Drawable>> GetDrawables();

	void AddAnimation(std::shared_ptr<MyAnimation> animation);

	BBox	m_bbox;
private:
    std::vector<std::shared_ptr<Drawable>> _drawables;
	std::vector<std::shared_ptr<MyAnimation>> m_animations;
};

