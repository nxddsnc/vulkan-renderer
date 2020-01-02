#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Mesh.h"
#pragma once
/************************************************************************/
/* Minimum renderable node.*/
/************************************************************************/
class MyNode
{
public:
    MyNode(glm::mat4 matrix);
    ~MyNode();

private:
    glm::mat4 m_matrix;
};

