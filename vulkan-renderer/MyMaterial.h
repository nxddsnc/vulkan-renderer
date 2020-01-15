#include <glm/glm.hpp>
#include <memory>

class MyTexture;
#pragma once
class MyMaterial
{
public:
    MyMaterial();
    ~MyMaterial();
private:
    glm::vec3                    _diffuse;
    float                        _transparent;
    std::shared_ptr<MyTexture*>  _diffuseMap;
    std::shared_ptr<MyTexture*>  _normalMap;
};

