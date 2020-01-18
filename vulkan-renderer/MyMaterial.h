#include <glm/glm.hpp>
#include <memory>
#include <string>
class MyTexture;
#pragma once
class MyMaterial
{
public:
    MyMaterial(std::string name);
    ~MyMaterial();
public:
    std::string                  m_name;
    glm::vec3                    m_diffuse;
    float                        m_opacity;
    std::shared_ptr<MyTexture>  m_pDiffuseMap;
    std::shared_ptr<MyTexture>  m_pNormalMap;
};

