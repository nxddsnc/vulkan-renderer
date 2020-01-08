#include "MyScene.h"

MyScene::MyScene()
{
}


MyScene::~MyScene()
{
}

void MyScene::AddDrawable(std::shared_ptr<Drawable> node)
{
    _drawables.push_back(node);
}

std::vector<std::shared_ptr<Drawable>> MyScene::GetDrawables()
{
    return _drawables;
}
