#include "Scene.h"



MyScene::MyScene()
{
}


MyScene::~MyScene()
{
}

void MyScene::AddNode(std::shared_ptr<RenderNode> node)
{
    _renderNodes.push_back(node);
}

std::vector<std::shared_ptr<RenderNode>> MyScene::GetNodes()
{
    return _renderNodes;
}
