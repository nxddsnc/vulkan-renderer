#include <glm/glm.hpp>
#include "Platform.h"
#include <array>
#include <vector>
#include "Drawable.h"
#include <memory.h>
#include <unordered_map>

#pragma once

class MyAnimation;
class MyScene
{
public:
	MyScene();
	~MyScene();

    void AddDrawable(std::shared_ptr<Drawable> node);
    std::vector<std::shared_ptr<Drawable>> GetDrawables();
	std::unordered_map<int, std::shared_ptr<Drawable>> GetDrawableMap() {return _drawableMap;};

	void AddAnimation(std::shared_ptr<MyAnimation> animation);

	BBox	m_bbox;

    std::vector<std::shared_ptr<MyAnimation>> m_animations;
private:
    std::vector<std::shared_ptr<Drawable>> _drawables;
	std::unordered_map<int, std::shared_ptr<Drawable>> _drawableMap;
	bool _instanceComputed;
};

