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
    glm::vec4                    m_baseColor;
    glm::vec2                    m_metallicRoughness;
    std::shared_ptr<MyTexture>   m_pDiffuseMap;
    std::shared_ptr<MyTexture>   m_pNormalMap;
    std::shared_ptr<MyTexture>   m_pMetallicRoughnessMap;
};

