#include <glm/glm.hpp>
#include "Platform.h"
#include <array>
#include <vector>
#include "Drawable.h"
#include <memory.h>
#pragma once

class MyScene
{
public:
	MyScene();
	~MyScene();

    void AddDrawable(std::shared_ptr<Drawable> node);
    std::vector<std::shared_ptr<Drawable>> GetDrawables();
private:
    std::vector<std::shared_ptr<Drawable>> _drawables;
};

