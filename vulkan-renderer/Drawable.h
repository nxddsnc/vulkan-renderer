#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "MyMesh.h"
#include "MyMaterial.h"
#include <memory.h>
#include "Platform.h"
#pragma once
/************************************************************************/
/* Minimum renderable node.*/
/************************************************************************/
struct Drawable
{
    glm::mat4                     matrix;
    std::shared_ptr<MyMesh>       mesh;
    std::shared_ptr<MyMaterial>   material;
    //vk::Buffer                    vertexBuffer;
    //VmaAllocation                 vertexBufferMemory;
    //vk::Buffer                    indexBuffer;
    //VmaAllocation                 indexBufferMemory;
};

