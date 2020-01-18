#include "MyMaterial.h"

MyMaterial::MyMaterial(std::string name)
{
    m_name          = name;
    m_diffuse.r     = 1.0;
    m_diffuse.g     = 1.0;
    m_diffuse.b     = 1.0;
    m_opacity       = 1.0;
    m_pDiffuseMap   = nullptr;
    m_pNormalMap    = nullptr;
}

MyMaterial::~MyMaterial()
{

}
