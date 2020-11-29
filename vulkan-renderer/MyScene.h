#include <glm/glm.hpp>
#include "Platform.h"
#include <array>
#include <vector>
#include "Renderable.h"
#include <memory.h>
#include <unordered_map>

#pragma once

class MyAnimation;
class MyScene
{
public:
	MyScene();
	~MyScene();

    void AddRenderable(std::shared_ptr<Renderable> node);
    std::vector<std::shared_ptr<Renderable>> GetRenderables();
	std::unordered_map<int, std::shared_ptr<Renderable>> GetRenderableMap() {return _renderableMap;};

	void AddAnimation(std::shared_ptr<MyAnimation> animation);

	BBox	m_bbox;

    std::vector<std::shared_ptr<MyAnimation>> m_animations;
private:
    std::vector<std::shared_ptr<Renderable>> _renderables;
	std::unordered_map<int, std::shared_ptr<Renderable>> _renderableMap;
	bool _instanceComputed;
};

