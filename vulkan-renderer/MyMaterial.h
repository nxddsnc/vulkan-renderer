#include <glm/glm.hpp>
class MyTexture;
#pragma once
class MyMaterial
{
public:
    MyMaterial();
    ~MyMaterial();
private:
    glm::vec3     _diffuse;
    float         _transparent;
    MyTexture   * _diffuseMap;
    MyTexture   * _normalMap;
};

