#include "MyMaterial.h"

MyMaterial::MyMaterial(std::string name)
{
    m_name                  = name;
    m_baseColor.r           = 1.0;
    m_baseColor.g           = 1.0;
    m_baseColor.b           = 1.0;
    m_baseColor.a           = 1.0;

	m_metallicRoughness     = glm::vec2(0, 1);
    m_pDiffuseMap           = nullptr;
    m_pNormalMap            = nullptr;
    m_pMetallicRoughnessMap = nullptr;
}

MyMaterial::~MyMaterial()
{

}
