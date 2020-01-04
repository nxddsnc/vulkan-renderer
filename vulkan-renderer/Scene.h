#include <glm/glm.hpp>
#include "Platform.h"
#include <array>
#include <vector>
#include "MyNode.h"
#include <memory.h>
#pragma once

class MyScene
{
public:
	MyScene();
	~MyScene();

    void AddNode(std::shared_ptr<RenderNode> node);
    std::vector<std::shared_ptr<RenderNode>> GetNodes();
private:
    std::vector<std::shared_ptr<RenderNode>> _renderNodes;
};

